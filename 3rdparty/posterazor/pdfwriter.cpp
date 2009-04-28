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

#include "paintcanvasinterface.h"
#include "pdfwriter.h"
#include <QRectF>
#include <QBrush>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#define LINEFEED "\x0A"

const int valuePrecision = 4;

#define COMPRESSEDPDF

static double cm2Pt(double cm)
{
    return Types::convertBetweenUnitsOfLength(cm, Types::UnitOfLengthCentimeter, Types::UnitOfLengthPoints);
}

PDFWriter::PDFWriter(QObject *parent)
    : QObject(parent)
    , m_outputFile(NULL)
    , m_pdfObjectCount(0)
    , m_objectPagesID(0)
    , m_firstPageID(5) // will be ++ed if the image has a SMask
    , m_objectResourcesID(0)
    , m_objectImageID(0)
    , m_mediaboxWidth(5000.0)
    , m_mediaboxHeight(5000.0)
{
}

void PDFWriter::addOffsetToXref()
{
    m_pdfObjectCount++;
    m_outStream.flush();
    m_xref.append(
        QString("%1 %2 n " LINEFEED)
        .arg((int)m_outStream.device()->size(), 10, 10, QLatin1Char('0'))
        .arg(0, 5, 10, QLatin1Char('0')));
}

int PDFWriter::addImageResourcesAndXObject()
{
    int err = 0;

    addOffsetToXref();
    m_objectResourcesID = m_pdfObjectCount;
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</XObject %2 0 R" LINEFEED
        "/ProcSet [/PDF /Text /ImageC /ImageI /ImageB]" LINEFEED
        ">>" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(m_pdfObjectCount + 1);

    addOffsetToXref();
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</Im1 %2 0 R" LINEFEED
        ">>" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(m_pdfObjectCount + 1);

    return err;
}

int PDFWriter::saveJpegImage(const QString &jpegFileName, const QSize &sizePixels, Types::ColorTypes colorType)
{
    int err = 0;

    err = addImageResourcesAndXObject();
    QFile jpegFile(jpegFileName);
    if (!jpegFile.open(QIODevice::ReadOnly))
        return 2;

    const int jpegFileSize = jpegFile.size();
    if (jpegFileSize == 0)
        return 3;

    const QString colorSpace =
        colorType==Types::ColorTypeCMYK?"/DeviceCMYK"
        :colorType==Types::ColorTypeRGB?"/DeviceRGB"
        :"/DeviceGray";

    // Yes. Cmyk jpegs in PDFs need reverse decoding, somehow
    const QString decodeArray =
        colorType==Types::ColorTypeCMYK?"/Decode [1.0 0.0 1.0 0.0 1.0 0.0 1.0 0.0]" LINEFEED
        :"";

    addOffsetToXref();
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</ColorSpace %2" LINEFEED
        "/Subtype /Image" LINEFEED
        "/Length %3" LINEFEED
        "/Width %4" LINEFEED
        "/Type /XObject" LINEFEED
        "/Height %5" LINEFEED
        "/BitsPerComponent 8" LINEFEED
        "/Filter /DCTDecode" LINEFEED
        "%6"
        ">>" LINEFEED
        "stream" LINEFEED)
        .arg(m_pdfObjectCount)
        .arg(colorSpace)
        .arg(jpegFileSize)
        .arg(sizePixels.width())
        .arg(sizePixels.height())
        .arg(decodeArray);

    m_outStream.flush();
    while (!jpegFile.atEnd())
        m_outStream.device()->write(jpegFile.read(200000));

    m_outStream <<
        LINEFEED "endstream" LINEFEED
        "endobj";

    return err;
}

int PDFWriter::saveImage(const QByteArray &imageData, const QSize &sizePixels, int bitPerPixel, Types::ColorTypes colorType, const QVector<QRgb> &colorTable)
{
    int err = 0;
    err = addImageResourcesAndXObject();
    const bool hasSoftMask = colorType == Types::ColorTypeRGBA;
    const Types::ColorTypes actualColorType = hasSoftMask?Types::ColorTypeRGB:colorType;
    const int actualBitsPerPixel = hasSoftMask?(bitPerPixel/4)*3:bitPerPixel;

    QString sMaskString;
    QByteArray softMask;
    QByteArray actualImageData;
    // Extract the alpha channel into "softMask"
    if (hasSoftMask) {
        const unsigned int pixelCount = sizePixels.height() * sizePixels.width();
        actualImageData.resize(imageData.size() - pixelCount);
        softMask.resize(pixelCount);
        const char *source = imageData.data();
        char *destinationRgb = actualImageData.data();
        char *destinationAlpha = softMask.data();
        for (unsigned int pixel = 0; pixel < pixelCount; pixel++){
            *destinationAlpha++ = *source++;
            *destinationRgb++ = *source++;
            *destinationRgb++ = *source++;
            *destinationRgb++ = *source++;
        }
        sMaskString = QString("/SMask %1 0 R" LINEFEED).arg(m_pdfObjectCount + 2);
    } else {
        actualImageData = imageData;
    }

#ifdef COMPRESSEDPDF
    const int compressedByteArrayPrependedBytes = 4;
    const int compressedByteArrayAppendedBytes = 4;
    const QByteArray imageDataCompressed = qCompress(actualImageData, 9);
    // qCompress adds 4 extra bytes before and after the compressed
    // results. In the prepended bytes, we have the original size
    // of the uncompressed data. We need to chop these bytes off
    // from both ends when inserting into the PDF document...
#endif

    QString colorSpaceString;
    switch (actualColorType) {
    case Types::ColorTypeRGB:
        colorSpaceString = "/DeviceRGB";
        break;
    case Types::ColorTypeGreyscale:
        colorSpaceString = "/DeviceGray";
        break;
    case Types::ColorTypeCMYK:
        colorSpaceString = "/DeviceCMYK";
        break;
    default:
        colorSpaceString = QString("[/Indexed /DeviceRGB %1 <").arg(colorTable.count()-1); // -1, because PDF wants the highest index, not the number of entries
        foreach (const QRgb &paletteEntry, colorTable) {
            QString rgbHex = QString("%1%2%3")
                .arg(qRed(paletteEntry), 2, 16, QLatin1Char('0'))
                .arg(qGreen(paletteEntry), 2, 16, QLatin1Char('0'))
                .arg(qBlue(paletteEntry), 2, 16, QLatin1Char('0'));
            colorSpaceString.append(rgbHex);
        }
        colorSpaceString.append(">]");
    }

    const int bitsPerComponent =
        actualColorType == Types::ColorTypePalette?actualBitsPerPixel
        :actualColorType == Types::ColorTypeMonochrome?actualBitsPerPixel
        :actualColorType == Types::ColorTypeGreyscale?actualBitsPerPixel
        :actualColorType == Types::ColorTypeCMYK?(actualBitsPerPixel/4)
        :(actualBitsPerPixel/3);
    addOffsetToXref();
    m_objectImageID = m_pdfObjectCount;
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</ColorSpace %2" LINEFEED
        "/Subtype /Image" LINEFEED
        "/Length %3" LINEFEED
        "/Width %4" LINEFEED
        "/Type /XObject" LINEFEED
        "/Height %5" LINEFEED
#ifdef COMPRESSEDPDF
        "/Filter /FlateDecode" LINEFEED
#endif
        "/BitsPerComponent %6" LINEFEED
        "%7"
        ">>" LINEFEED
        "stream" LINEFEED)
        .arg(m_pdfObjectCount)
        .arg(colorSpaceString)
#ifdef COMPRESSEDPDF
        .arg(imageDataCompressed.size() - compressedByteArrayPrependedBytes - compressedByteArrayAppendedBytes)
#else
        .arg(imageData.size())
#endif
        .arg(sizePixels.width())
        .arg(sizePixels.height())
        .arg(bitsPerComponent)
        .arg(sMaskString);

    m_outStream.flush(); // Important to flush stream before writing to device
    m_outStream.device()->write(
#ifdef COMPRESSEDPDF
        imageDataCompressed.constData() + compressedByteArrayPrependedBytes,
        imageDataCompressed.size() - compressedByteArrayPrependedBytes - compressedByteArrayAppendedBytes
#else
        imageData.constData(), imageData.size()
#endif
    );
    m_outStream <<
        LINEFEED "endstream" LINEFEED
        "endobj";

    if (hasSoftMask) {
#ifdef COMPRESSEDPDF
        const QByteArray softMaskDataCompressed = qCompress(softMask, 9);
#endif
        addOffsetToXref();
        m_outStream << QString(
            LINEFEED "%1 0 obj" LINEFEED
            "<</ColorSpace /DeviceGray" LINEFEED
            "/Subtype /Image" LINEFEED
            "/Length %2" LINEFEED
            "/Width %3" LINEFEED
            "/Type /XObject" LINEFEED
            "/Height %4" LINEFEED
#ifdef COMPRESSEDPDF
            "/Filter /FlateDecode" LINEFEED
#endif
            "/BitsPerComponent 8" LINEFEED
            "/Decode [ 0 1 ]" LINEFEED
            ">>" LINEFEED
            "stream" LINEFEED)
            .arg(m_pdfObjectCount)
#ifdef COMPRESSEDPDF
            .arg(softMaskDataCompressed.size() - compressedByteArrayPrependedBytes - compressedByteArrayAppendedBytes)
#else
            .arg(softMask.size())
#endif
            .arg(sizePixels.width())
            .arg(sizePixels.height());
        m_outStream.flush(); // Important to flush stream before writing to device
        m_outStream.device()->write(
#ifdef COMPRESSEDPDF
            softMaskDataCompressed.constData() + compressedByteArrayPrependedBytes,
            softMaskDataCompressed.size() - compressedByteArrayPrependedBytes - compressedByteArrayAppendedBytes
#else
            softMask.constData(), softMask.size()
#endif
        );
        m_outStream <<
            LINEFEED "endstream" LINEFEED
            "endobj";
        m_firstPageID++;
    }

    return err;
}

int PDFWriter::startPage()
{
    int err = 0;

    m_pageContent.clear();
    addOffsetToXref();
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</Group <</CS /DeviceRGB" LINEFEED
        "/I true" LINEFEED
        "/S /Transparency" LINEFEED
        ">>" LINEFEED
        "/Parent %2 0 R" LINEFEED
        "/MediaBox [0 0 %3 %4]" LINEFEED
        "/Resources %5 0 R" LINEFEED
        "/Contents %6 0 R" LINEFEED
        "/Type /Page" LINEFEED
        ">>" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(m_firstPageID + m_contentPagesCount*2)
        .arg(m_mediaboxWidth, 0, 'f', valuePrecision)
        .arg(m_mediaboxHeight, 0, 'f', valuePrecision)
        .arg(m_objectResourcesID)
        .arg(m_pdfObjectCount+1);

    return err;
}

int PDFWriter::finishPage()
{
    int err = 0;

    addOffsetToXref();
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</Length %2" LINEFEED
        ">>" LINEFEED
        "stream" LINEFEED
        "%3" LINEFEED
        "endstream" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(m_pageContent.length())
        .arg(m_pageContent);

    return err;
}

int PDFWriter::startSaving(const QString &fileName, int pages, double widthCm, double heightCm)
{
    int err = 0;

    m_mediaboxWidth = cm2Pt(widthCm);
    m_mediaboxHeight = cm2Pt(heightCm);

    if (m_outputFile) {
        m_outputFile->close();
        delete m_outputFile;
    }
    m_outputFile = new QFile(fileName, this);
    if (!m_outputFile->open(QIODevice::WriteOnly))
        return 1;

    m_outStream.setDevice(m_outputFile);
    m_contentPagesCount = pages;
    m_xref.clear();
    m_outStream << "%PDF-1.3" LINEFEED
        "%\xe2\xe3\xcf\xd3" ;

    addOffsetToXref();
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</Creator (PosteRazor)" LINEFEED
        "/Producer (PosteRazor.SourceForge.net)" LINEFEED
        "/CreationDate (D:%2)" LINEFEED
        ">>" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));

    return err;
}

int PDFWriter::finishSaving()
{
    int err = 0;

    addOffsetToXref();
    m_objectPagesID = m_pdfObjectCount;
    QString kids;
    for (int i = 0; i < m_contentPagesCount; i++)
        kids.append(QString("%1%2 0 R").arg(i != 0?" ":"").arg(i*2 + m_firstPageID));
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</MediaBox [0 0 %2 %3]" LINEFEED
        "/Resources %4 0 R" LINEFEED
        "/Kids [%5]" LINEFEED
        "/Count %6" LINEFEED
        "/Type /Pages" LINEFEED
        ">>" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(m_mediaboxWidth, 0, 'f', valuePrecision)
        .arg(m_mediaboxHeight, 0, 'f', valuePrecision)
        .arg(m_objectResourcesID)
        .arg(kids)
        .arg(m_contentPagesCount);

    addOffsetToXref();
    m_outStream << QString(
        LINEFEED "%1 0 obj" LINEFEED
        "<</Pages %2 0 R" LINEFEED
        "/Type /Catalog" LINEFEED
        ">>" LINEFEED
        "endobj")
        .arg(m_pdfObjectCount)
        .arg(m_pdfObjectCount - 1);

    m_outStream.flush();
    const qint64 startxref = m_outStream.device()->size();
    m_outStream
        << QString(LINEFEED "xref" LINEFEED "0 %1" LINEFEED "0000000000 65535 f " LINEFEED)
        .arg(m_pdfObjectCount + 1)
        << m_xref
        << QString(
        "trailer" LINEFEED
        "<</Info %1 0 R" LINEFEED
        "/Root %2 0 R" LINEFEED
        "/Size %3" LINEFEED
        ">>" LINEFEED
        "startxref" LINEFEED
        "%4" LINEFEED
        "%%EOF" LINEFEED)
        .arg(1)
        .arg(m_pdfObjectCount)
        .arg(m_pdfObjectCount + 1)
        .arg(startxref);

    m_xref.clear();
    return err;
}

void PDFWriter::drawFilledRect(const QRectF& rect, const QBrush &brush)
{
    Q_UNUSED(rect)
    Q_UNUSED(brush)
}

QSizeF PDFWriter::size() const
{
    return QSizeF();
}

void PDFWriter::drawImage(const QRectF &rect)
{
    const QString imageCode = QString(
        "0 w" LINEFEED
        "q 0 0 %1 %2 re W* n" LINEFEED
        "q %3 0 0 %4 %5 %6 cm" LINEFEED
        "  /Im1 Do Q" LINEFEED
        "Q ")
        .arg(m_mediaboxWidth, 0, 'f', valuePrecision)
        .arg(m_mediaboxHeight, 0, 'f', valuePrecision)
        .arg(cm2Pt(rect.width()), 0, 'f', valuePrecision)
        .arg(cm2Pt(rect.height()), 0, 'f', valuePrecision)
        .arg(cm2Pt(rect.x()), 0, 'f', valuePrecision)
        .arg(m_mediaboxHeight-cm2Pt(rect.y())-cm2Pt(rect.height()), 0, 'f', valuePrecision);

    m_pageContent.append(imageCode);
}

void PDFWriter::drawOverlayText(const QPointF &position, int flags, int size, const QString &text)
{
    Q_UNUSED(position);
    Q_UNUSED(flags);
    Q_UNUSED(size);
    Q_UNUSED(text);
}
