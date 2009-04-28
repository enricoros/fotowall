/*
    PosteRazor - Make your own poster!
    Copyright (C) 2005-2009 by Alessandro Portale
    http://posterazor.sourceforge.net/

    This file is part of PosteRazor

    PosteRazor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PosteRazor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PosteRazor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "FreeImage.h"
#include "imageloaderfreeimage.h"
#include <qendian.h>
#include <QStringList>
#include <QColor>
#include <math.h>

static QString FreeImageErrorMessage;

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
{
    Q_UNUSED(fif)
    FreeImageErrorMessage = message;
}

class FreeImageInitializer
{
public:
    FreeImageInitializer()
    {
        FreeImage_Initialise();
        FreeImage_SetOutputMessage(FreeImageErrorHandler);
    }

    ~FreeImageInitializer()
    {
        FreeImage_DeInitialise();
    }
};

ImageLoaderFreeImage::ImageLoaderFreeImage(QObject *parent)
    : QObject(parent)
    , m_bitmap(NULL)
    , m_widthPixels(0)
    , m_heightPixels(0)
    , m_horizontalDotsPerMeter(0)
    , m_verticalDotsPerMeter(0)
{
    const static FreeImageInitializer initializer;
}

ImageLoaderFreeImage::~ImageLoaderFreeImage()
{
    disposeImage();
}

void ImageLoaderFreeImage::disposeImage()
{
    if (m_bitmap) {
        FreeImage_Unload(m_bitmap);
        m_bitmap = NULL;
    }
}

bool ImageLoaderFreeImage::loadInputImage(const QString &imageFileName, QString &errorMessage)
{
    bool result = false;

    FreeImageErrorMessage.clear();

    const FREE_IMAGE_FORMAT fileType = FreeImage_GetFileType(imageFileName.toAscii(), 0);
    FIBITMAP* newImage = FreeImage_Load(fileType, imageFileName.toAscii(), TIFF_CMYK|JPEG_CMYK);

    // Filter out images which FreeImage can load but not convert to Rgb24
    // And images which we simply don't handle
    if (newImage) {
        const FREE_IMAGE_TYPE type = FreeImage_GetImageType(newImage);
        if (type != FIT_BITMAP   // 1pbb Monochrome, 1-8bpp Palette, 8bpp Greyscale,
                                 // 24bpp Rgb, 32bpp Argb, 32bpp Cmyk
            && type != FIT_RGB16 // 16bpp Greyscale, 48bpp Rgb
            ) {
            FreeImage_Unload(newImage);
            newImage = NULL;
        }
    }

    if (newImage) {
        result = true;
        disposeImage();

        m_bitmap = newImage;

        m_widthPixels = FreeImage_GetWidth(m_bitmap);
        m_heightPixels = FreeImage_GetHeight(m_bitmap);

        m_horizontalDotsPerMeter = FreeImage_GetDotsPerMeterX(m_bitmap);
        m_verticalDotsPerMeter = FreeImage_GetDotsPerMeterY(m_bitmap);

        if (m_horizontalDotsPerMeter == 0)
            m_horizontalDotsPerMeter = 2835; // 72 dpi
        if (m_verticalDotsPerMeter == 0)
            m_verticalDotsPerMeter = 2835;

        m_imageFileName = imageFileName;

        if (colorDataType() == Types::ColorTypeRGB && bitsPerPixel() == 32) {
            // Sometimes, there are strange .PSD images like this (FreeImage bug?)
            RGBQUAD white = { 255, 255, 255, 0 };
            FIBITMAP *Image24Bit = FreeImage_Composite(m_bitmap, FALSE, &white);
            FreeImage_Unload(m_bitmap);
            m_bitmap = Image24Bit;
        }
    }

    errorMessage = FreeImageErrorMessage;

    return result;
}

bool ImageLoaderFreeImage::isImageLoaded() const
{
    return (m_bitmap != NULL);
}

bool ImageLoaderFreeImage::isJpeg() const
{
    return FreeImage_GetFileType(m_imageFileName.toAscii(), 0) == FIF_JPEG;
}

QString ImageLoaderFreeImage::fileName() const
{
    return m_imageFileName;
}

QSize ImageLoaderFreeImage::sizePixels() const
{
    return QSize(m_widthPixels, m_heightPixels);
}

double ImageLoaderFreeImage::horizontalDotsPerUnitOfLength(Types::UnitsOfLength unit) const
{
    return m_horizontalDotsPerMeter / Types::convertBetweenUnitsOfLength(1, Types::UnitOfLengthMeter, unit);
}

double ImageLoaderFreeImage::verticalDotsPerUnitOfLength(Types::UnitsOfLength unit) const
{
    return m_verticalDotsPerMeter / Types::convertBetweenUnitsOfLength(1, Types::UnitOfLengthMeter, unit);
}

QSizeF ImageLoaderFreeImage::size(Types::UnitsOfLength unit) const
{
    const QSize sizePixels = this->sizePixels();
    return QSizeF(sizePixels.width() / horizontalDotsPerUnitOfLength(unit), sizePixels.height() / verticalDotsPerUnitOfLength(unit));
}

const QImage ImageLoaderFreeImage::imageAsRGB(const QSize &size) const
{
    const QSize resultSize = size.isValid()?size:sizePixels();
    const bool isRGB24 = colorDataType() == Types::ColorTypeRGB && bitsPerPixel() == 24;
    const bool isARGB32 = colorDataType() == Types::ColorTypeRGBA && bitsPerPixel() == 32;
    QImage result(resultSize, isARGB32?QImage::Format_ARGB32:QImage::Format_RGB32);

    const int width = resultSize.width();
    const int height = resultSize.height();
    const QSize sizePixels = this->sizePixels();

    FIBITMAP* originalImage = m_bitmap;
    FIBITMAP* temp24BPPImage = NULL;
    FIBITMAP* scaledImage = NULL;

    if (!(isRGB24 || isARGB32)) {
        if (colorDataType() == Types::ColorTypeCMYK) {
            const bool isCmykJpeg = isJpeg(); // Value range inverted
            temp24BPPImage = FreeImage_Allocate(sizePixels.width(), sizePixels.height(), 24);
            const unsigned int columnsCount = sizePixels.width();
            const unsigned int scanlinesCount = sizePixels.height();
            for (unsigned int scanline = 0; scanline < scanlinesCount; scanline++) {
                const BYTE* const cmykBits = FreeImage_GetScanLine(m_bitmap, scanline);
                tagRGBTRIPLE* const rgbBits = (tagRGBTRIPLE *)FreeImage_GetScanLine(temp24BPPImage, scanline);

                for (unsigned int column = 0; column < columnsCount; column++) {
                    const unsigned int cmykColumn = column * 4;

                    const QColor rgbColor = isCmykJpeg?
                        QColor::fromCmyk(255 - cmykBits[cmykColumn], 255 - cmykBits[cmykColumn + 1], 255 - cmykBits[cmykColumn + 2], 255 - cmykBits[cmykColumn + 3])
                        :QColor::fromCmyk(cmykBits[cmykColumn], cmykBits[cmykColumn + 1], cmykBits[cmykColumn + 2], cmykBits[cmykColumn + 3]);

                    rgbBits[column].rgbtRed = (BYTE)rgbColor.red();
                    rgbBits[column].rgbtGreen = (BYTE)rgbColor.green();
                    rgbBits[column].rgbtBlue = (BYTE)rgbColor.blue();
                }
            }
        } else {
            temp24BPPImage = FreeImage_ConvertTo24Bits(originalImage);
        }
        originalImage = temp24BPPImage;
    }

    if (resultSize != sizePixels) {
        scaledImage = FreeImage_Rescale(originalImage, width, height, FILTER_BOX);
        originalImage = scaledImage;
    }

    for (int scanline = 0; scanline < height; scanline++) {
        QRgb *targetData = (QRgb*)result.scanLine(scanline);
        if (isARGB32) {
            const tagRGBQUAD *sourceRgba = (tagRGBQUAD*)FreeImage_GetScanLine(originalImage, height - scanline - 1);
            for (int column = 0; column < width; column++) {
                *targetData++ = qRgba(sourceRgba->rgbRed, sourceRgba->rgbGreen, sourceRgba->rgbBlue, sourceRgba->rgbReserved);
                sourceRgba++;
            }
        } else {
            const tagRGBTRIPLE *sourceRgb = (tagRGBTRIPLE*)FreeImage_GetScanLine(originalImage, height - scanline - 1);
            for (int column = 0; column < width; column++) {
                *targetData++ = qRgb(sourceRgb->rgbtRed, sourceRgb->rgbtGreen, sourceRgb->rgbtBlue);
                sourceRgb++;
            }
        }
    }

    if (temp24BPPImage)
        FreeImage_Unload(temp24BPPImage);

    if (scaledImage)
        FreeImage_Unload(scaledImage);

    return result;
}

int ImageLoaderFreeImage::bitsPerPixel() const
{
    return FreeImage_GetBPP(m_bitmap);
}

Types::ColorTypes ImageLoaderFreeImage::colorDataType() const
{
    Types::ColorTypes colorDatatype = Types::ColorTypeRGB;
    const FREE_IMAGE_COLOR_TYPE imageColorType = FreeImage_GetColorType(m_bitmap);

    if (imageColorType == FIC_MINISBLACK || imageColorType == FIC_MINISWHITE) {
        colorDatatype = bitsPerPixel()==1?Types::ColorTypeMonochrome:Types::ColorTypeGreyscale;
    } else {
        colorDatatype =
            imageColorType==FIC_PALETTE?Types::ColorTypePalette:
            imageColorType==FIC_RGB?Types::ColorTypeRGB:
            imageColorType==FIC_RGBALPHA?Types::ColorTypeRGBA:
            /*imageColorType==FIC_CMYK?*/Types::ColorTypeCMYK;
    }

    return colorDatatype;
}

const QByteArray ImageLoaderFreeImage::bits() const
{
    const unsigned int bitsPerLine = m_widthPixels * bitsPerPixel();
    const unsigned int bytesPerLine = (unsigned int)ceil(bitsPerLine/8.0);
    const unsigned int imageBytesCount = bytesPerLine * m_heightPixels;

    QByteArray result(imageBytesCount, 0);
    char *destination = result.data();
    FreeImage_ConvertToRawBits((BYTE*)destination, m_bitmap, bytesPerLine, bitsPerPixel(), FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, hasFreeImageVersionCorrectTopDownInConvertBits());

    const unsigned long numberOfPixels = m_widthPixels * m_heightPixels;
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
    if (colorDataType() == Types::ColorTypeRGB && bitsPerPixel() == 24) {
        for (unsigned int pixelIndex = 0; pixelIndex < numberOfPixels; pixelIndex++) {
            char *pixelPtr = destination + pixelIndex*3;
            const char temp = pixelPtr[0];
            pixelPtr[0] = pixelPtr[2];
            pixelPtr[2] = temp;
            pixelPtr+=3;
        }
    } else if (colorDataType() == Types::ColorTypeRGBA && bitsPerPixel() == 32) {
        unsigned int* argbDestination = (unsigned int*)destination;
        for (unsigned int pixelIndex = 0; pixelIndex < numberOfPixels; pixelIndex++)
            *argbDestination++ = qToBigEndian(*argbDestination);
    } else
#endif // FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
    if (colorDataType() == Types::ColorTypeRGB && bitsPerPixel() == 48) {
        // Apparently, the 48 bit data has to be reordered on Windows and ppc/i386 OSX
        // TODO: So maybe this swap belongs into the PDFwriter. Investigate.
        unsigned short* rgb48Destination = (unsigned short*)destination;
        const unsigned long numberOfSwaps = numberOfPixels * 3; // Words are swapped
        for (unsigned int pixelIndex = 0; pixelIndex < numberOfSwaps; pixelIndex++)
            *rgb48Destination++ = qToBigEndian(*rgb48Destination);
    }

    return result;
}

const QVector<QRgb> ImageLoaderFreeImage::colorTable() const
{
    QVector<QRgb> result;

    const RGBQUAD* const palette = FreeImage_GetPalette(m_bitmap);
    if (palette) {
        const int count = FreeImage_GetColorsUsed(m_bitmap);
        result.resize(count);
        for (int i = 0; i < count; i++)
            result.replace(i, qRgb(palette[i].rgbRed, palette[i].rgbGreen, palette[i].rgbBlue));
    }

    return result;
}

const QVector<QPair<QStringList, QString> > &ImageLoaderFreeImage::imageFormats() const
{
    static QVector<QPair<QStringList, QString> > formats;
    if (formats.empty()) {
        const int pluginsCount = FreeImage_GetFIFCount();
        for (int pluginIndex = 0; pluginIndex < pluginsCount; pluginIndex++) {
            const FREE_IMAGE_FORMAT fif = (FREE_IMAGE_FORMAT)pluginIndex;
            if (FreeImage_FIFSupportsReading(fif)) {
                const QString pluginExtensions(FreeImage_GetFIFExtensionList(fif));
                const QString pluginDescription(FreeImage_GetFIFDescription(fif));
                formats.append(QPair<QStringList, QString> (pluginExtensions.split(','), pluginDescription));
            }
        }
    }
    return formats;
}

bool ImageLoaderFreeImage::hasFreeImageVersionCorrectTopDownInConvertBits()
{
    return FREEIMAGE_MAJOR_VERSION >= 3 && FREEIMAGE_MINOR_VERSION >= 10;
}
QString ImageLoaderFreeImage::libraryName() const
{
    return QLatin1String("FreeImage");
}

QString ImageLoaderFreeImage::libraryAboutText() const
{
    static const QString copyrightMessage(FreeImage_GetCopyrightMessage());
    return copyrightMessage;
}
