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

#ifndef IMAGELOADERINTERFACE_H
#define IMAGELOADERINTERFACE_H

#include "types.h"
#include <QRgb>
#include <QImage>
#include <QSize>

class PainterInterface;

class ImageLoaderInterface
{
public:
    virtual ~ImageLoaderInterface() {}

    virtual bool loadInputImage(const QString &imageFileName, QString &errorMessage) = 0;
    virtual bool isImageLoaded() const = 0;
    virtual bool isJpeg() const = 0;
    virtual QString fileName() const = 0;
    virtual QSize sizePixels() const = 0;
    virtual double horizontalDotsPerUnitOfLength(Types::UnitsOfLength unit) const = 0;
    virtual double verticalDotsPerUnitOfLength(Types::UnitsOfLength unit) const = 0;
    virtual QSizeF size(Types::UnitsOfLength unit) const = 0;
    virtual const QImage imageAsRGB(const QSize &size) const = 0;
    virtual int bitsPerPixel() const = 0;
    virtual Types::ColorTypes colorDataType() const = 0;
    virtual const QByteArray bits() const = 0;
    virtual const QVector<QRgb> colorTable() const = 0;
    virtual const QVector<QPair<QStringList, QString> > &imageFormats() const = 0;
    virtual QString libraryName() const = 0;
    virtual QString libraryAboutText() const = 0;
};

#endif // IMAGELOADERINTERFACE_H
