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

#ifndef PDFWRITER_H
#define PDFWRITER_H

#include "types.h"
#include "paintcanvasinterface.h"
#include <QVector>
#include <QRgb>
#include <QSize>
#include <QTextStream>

class QFile;

class PDFWriter: public QObject, public PaintCanvasInterface
{
public:
    PDFWriter(QObject *parent = 0);

    void addOffsetToXref();
    int addImageResourcesAndXObject();
    int saveJpegImage(const QString &jpegFileName, const QSize &sizePixels, Types::ColorTypes colorType);
    int saveImage(const QByteArray &imageData, const QSize &sizePixels, int bitPerPixel, Types::ColorTypes colorType, const QVector<QRgb> &colorTable);
    int startPage();
    int finishPage();
    int startSaving(const QString &fileName, int pages, double widthCm, double heightCm);
    int finishSaving();
    void drawFilledRect(const QRectF&, const QBrush &brush);
    QSizeF size() const;
    void drawImage(const QRectF &rect);
    void drawOverlayText(const QPointF &position, int flags, int size, const QString &text);

private:
    QFile *m_outputFile;
    QString m_xref;
    int m_pdfObjectCount;
    int m_contentPagesCount;
    int m_objectPagesID;
    int m_firstPageID;
    int m_objectResourcesID;
    int m_objectImageID;
    double m_mediaboxWidth;
    double m_mediaboxHeight;
    QString m_pageContent;
    QTextStream m_outStream;
};

#endif // PDFWRITER_H
