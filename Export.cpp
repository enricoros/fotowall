/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Tanguy Arnaud <arn.tanguy@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QPrinter>
#include <QPrintDialog>
#include "Export.h"
#include "Desk.h"

Export::Export(Desk *desk) : m_desk(desk)
{
    m_ui.setupUi(this);
    // Set default sizes
    m_ui.saveHeight->setValue(m_desk->height());
    m_ui.saveWidth->setValue(m_desk->width());
    m_ui.printWidth->setValue(m_desk->width());
    m_ui.printHeight->setValue(m_desk->height());
    m_printSize.setWidth(m_desk->width()/m_ui.printDpi->value());
    m_printSize.setHeight(m_desk->height()/m_ui.printDpi->value());

    connect(m_ui.saveButton, SIGNAL(clicked()), this, SLOT(slotSaveImage()));
    connect(m_ui.chooseFilePath, SIGNAL(clicked()), this, SLOT(slotChoosePath()));

    connect(m_ui.printButton, SIGNAL(clicked()), this, SLOT(slotPrint()));
    connect(m_ui.printUnity, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintUnityChanged(int)));
    connect(m_ui.printWidth, SIGNAL(valueChanged(double)), this, SLOT(slotPrintWidthChanged(double)));
    connect(m_ui.printHeight, SIGNAL(valueChanged(double)), this, SLOT(slotPrintHeightChanged(double)));

    connect(m_ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void Export::slotSaveImage()
{
        if(m_ui.filePath->text().isEmpty()) {
            QMessageBox::warning(this, tr("No file selected !"), tr("You need to choose a file path for saving."));
            return;
        }
        QString fileName = m_ui.filePath->text();

        // get the rendering size
        int destW = m_ui.saveWidth->value();
        int destH = m_ui.saveHeight->value();

        // render on the image
        QImage image(destW, destH, QImage::Format_ARGB32);
        image.fill(0);
        QPainter imagePainter(&image);
        imagePainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        m_desk->renderVisible(&imagePainter, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
        imagePainter.end();

        if(m_ui.saveLandscape->isChecked()) {
            // Save in landscape mode, so rotate
            QMatrix matrix;
            matrix.rotate(90);
            image = image.transformed(matrix);
        }

        // save image
        if (!image.save(fileName) || !QFile::exists(fileName)) {
            QMessageBox::warning(this, tr("Rendering Error"), tr("Error rendering to the file '%1'").arg(fileName));
            return;
        }
        int size = QFile(fileName).size();
        QMessageBox::information(this, tr("Done"), tr("The target image is %1 bytes long").arg(size));

        setBackground();
}

void Export::slotChoosePath()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose the Image file"), m_ui.filePath->text(), tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if(fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += ".png";
    m_ui.filePath->setText(fileName);
}

void Export::slotPrint()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer);
    bool ok = printDialog.exec();
    if(!ok) return;

    float w = m_ui.printWidth->value(), h=m_ui.printHeight->value();
    int dpi = m_ui.printDpi->value();
    float fdpi = (float)dpi;
    if(m_ui.printUnity->currentIndex() == 0) { // If pixels
        m_printSize.setWidth(w/dpi);
        m_printSize.setHeight(h/dpi);
    } else if (m_ui.printUnity->currentIndex() == 1) { // If cm
        m_printSize.setWidth(w/2.54); //Convert in inches
        m_printSize.setHeight(h/2.54);
    } else if (m_ui.printUnity->currentIndex() == 2) { //If inches
        m_printSize.setWidth(w);
        m_printSize.setHeight(h);
    }


    QImage image(m_desk->width(), m_desk->height(), QImage::Format_ARGB32);
    image.fill(0);
    QPainter paintimg(&image);
    paintimg.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    // Render on the image
    m_desk->renderVisible(&paintimg, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    paintimg.end();
    QSize scaleSize(m_printSize.width()*fdpi, m_printSize.height()*fdpi);
    image = image.scaled(scaleSize);

    printer.setResolution(dpi);
    printer.setPaperSize(QPrinter::A4);

    if(m_ui.printLandscape->isChecked()) {
        // Print in landscape mode, so rotate
        QMatrix matrix;
        matrix.rotate(90);
        image = image.transformed(matrix);
    }

    QPainter paint(&printer);
    paint.drawImage(image.rect(), image);
}

void Export::slotPrintUnityChanged(int index)
{
    if(index == 0) {
        m_ui.printWidth->setValue(m_printSize.width() * m_ui.printDpi->value());
        m_ui.printHeight->setValue(m_printSize.height() * m_ui.printDpi->value());
    }
    if(index == 1) { //convert to cm
         m_ui.printWidth->setValue(m_printSize.width() * 2.54);
         m_ui.printHeight->setValue(m_printSize.height() * 2.54);
    } else if (index == 2) {
         m_ui.printWidth->setValue(m_printSize.width());
         m_ui.printHeight->setValue(m_printSize.height());
    }
}

void Export::slotPrintWidthChanged(double newWidth)
{
    qDebug() << "printWidth :"<<newWidth;
    switch(m_ui.printUnity->currentIndex()) {
        case 0: // Convert pixels to inches
            m_printSize.setWidth(newWidth/m_ui.printDpi->value());
            break;
        case 1: //Convert cm to inches
            m_printSize.setWidth(newWidth/2.54);
            break;
        case 2:
            m_printSize.setWidth(m_ui.printWidth->value());
            break;
    }
}
void Export::slotPrintHeightChanged(double newHeight)
{
    switch(m_ui.printUnity->currentIndex()) {
        case 0: // Convert pixels to inches
            m_printSize.setHeight(newHeight/m_ui.printDpi->value());
            break;
        case 1: //Convert cm to inches
            m_printSize.setHeight(newHeight/2.54);
            break;
        case 2:
            m_printSize.setHeight(m_ui.printHeight->value());
            break;
    }
}

void Export::setBackground()
{
if(!m_ui.setBackground->isChecked()) return;
#ifdef Q_OS_WINDOWS
    // XXX : Try that stuff on windows
#include <windows.h>

    QSettings appSettings( "HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);

    //Set new background path
    QString filePath = m_ui.filePath->text();
    appSettings.setValue("Wallpaper", filePath);
    QByteArray ba = filePath.toLatin1();
    //Notification to windows renew desktop
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(),	SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif

    // FIXME : how to set the background on linux?
}
