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

#include "ExportWizard.h"
#include "ui_ExportWizard.h"
#include "Desk.h"
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QUrl>

#define POSTERAZOR_WEBSITE_LINK "http://posterazor.sourceforge.net/"
#define POSTERAZOR_TUTORIAL_LINK "http://www.youtube.com/watch?v=p7XsFZ4Leo8"

ExportWizard::ExportWizard(Desk * desk)
    : QWizard()
    , m_ui(new Ui::ExportWizard)
    , m_desk(desk)
    , m_nextId(0)
{
    // create and init UI
    m_ui->setupUi(this);
    connect(m_ui->clWallpaper, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clImage, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clPosteRazor, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clPrint, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    m_ui->prWebLabel->setText("<html><body><a href='" POSTERAZOR_WEBSITE_LINK "'>" + m_ui->prWebLabel->text() + "</a></body></html>" );
    connect(m_ui->prWebLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(slotOpenLink(const QString &)));
    m_ui->prTutorialLabel->setText("<html><body><a href='" POSTERAZOR_TUTORIAL_LINK "'>" + m_ui->prTutorialLabel->text() + "</a></body></html>" );
    connect(m_ui->prTutorialLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(slotOpenLink(const QString &)));

    // set default sizes
    m_ui->saveHeight->setValue(m_desk->height());
    m_ui->saveWidth->setValue(m_desk->width());
    m_ui->printWidth->setValue(m_desk->width());
    m_ui->printHeight->setValue(m_desk->height());
    m_printSize.setWidth(m_desk->width()/m_ui->printDpi->value());
    m_printSize.setHeight(m_desk->height()/m_ui->printDpi->value());

    // connect buttons
    connect(m_ui->chooseFilePath, SIGNAL(clicked()), this, SLOT(slotChoosePath()));
    connect(m_ui->printUnity, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintUnityChanged(int)));
    connect(m_ui->printWidth, SIGNAL(valueChanged(double)), this, SLOT(slotPrintWidthChanged(double)));
    connect(m_ui->printHeight, SIGNAL(valueChanged(double)), this, SLOT(slotPrintHeightChanged(double)));

    // configure Wizard
    setOptions(NoDefaultButton | NoBackButtonOnStartPage | IndependentPages);
    setPage(PageMode);

    // react to 'finish'
    connect(this, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
}

ExportWizard::~ExportWizard()
{
    delete m_ui;
}

#if defined(Q_OS_WINDOWS)
#include <windows.h>
#endif
void ExportWizard::setWallpaper()
{
#if defined(Q_OS_WINDOWS)
    // XXX : Try that stuff on windows
    QSettings appSettings( "HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);

    //Set new background path
    QString filePath = m_ui->filePath->text();
    appSettings.setValue("Wallpaper", filePath);
    QByteArray ba = filePath.toLatin1();
    //Notification to windows renew desktop
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(),	SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#elif defined(Q_OS_LINUX)
    // FIXME : how to set the background on linux?
#else
    // handle other systems
#endif
}

void ExportWizard::saveImage()
{
    if(m_ui->filePath->text().isEmpty()) {
        QMessageBox::warning(this, tr("No file selected !"), tr("You need to choose a file path for saving."));
        return;
    }
    QString fileName = m_ui->filePath->text();

    // get the rendering size
    int destW = m_ui->saveWidth->value();
    int destH = m_ui->saveHeight->value();

    // render on the image
    QImage image(destW, destH, QImage::Format_ARGB32);
    image.fill(0);
    QPainter imagePainter(&image);
    imagePainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    m_desk->renderVisible(&imagePainter, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    imagePainter.end();

    if(m_ui->saveLandscape->isChecked()) {
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
}

void ExportWizard::startPosterazor()
{
    qWarning("3");
}

void ExportWizard::print()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer);
    bool ok = printDialog.exec();
    if(!ok) return;

    float w = m_ui->printWidth->value(), h=m_ui->printHeight->value();
    int dpi = m_ui->printDpi->value();
    float fdpi = (float)dpi;
    if(m_ui->printUnity->currentIndex() == 0) { // If pixels
        m_printSize.setWidth(w/dpi);
        m_printSize.setHeight(h/dpi);
    } else if (m_ui->printUnity->currentIndex() == 1) { // If cm
        m_printSize.setWidth(w/2.54); //Convert in inches
        m_printSize.setHeight(h/2.54);
    } else if (m_ui->printUnity->currentIndex() == 2) { //If inches
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

    if(m_ui->printLandscape->isChecked()) {
        // Print in landscape mode, so rotate
        QMatrix matrix;
        matrix.rotate(90);
        image = image.transformed(matrix);
    }

    QPainter paint(&printer);
    paint.drawImage(image.rect(), image);
}

void ExportWizard::setPage(int pageId)
{
    // adapt buttons
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::BackButton;
    if (pageId >= PageWallpaper && pageId <= PagePrint)
        layout << QWizard::FinishButton;
    layout << QWizard::CancelButton;
    setButtonLayout(layout);

    // change page
    m_nextId = pageId;
    next();

    // execute on-entry code
    // ...
}

int ExportWizard::nextId() const
{
    // dynamic page ordering
    const int pageId = currentId();

    // mode selection: return the id of the next page (set by the linkbuttons)
    if (pageId == PageMode)
        return m_nextId;

    // final pages
    if (pageId >= PageWallpaper && pageId <= PagePrint)
        return -1;

    // fallback
    qWarning("ExportWizard::nextId: unhandled nextId for page %d", pageId);
    return -1;
}

void ExportWizard::slotChoosePath()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose the Image file"), m_ui->filePath->text(), tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if(fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += ".png";
    m_ui->filePath->setText(fileName);
}

void ExportWizard::slotPrintUnityChanged(int index)
{
    if(index == 0) {
        m_ui->printWidth->setValue(m_printSize.width() * m_ui->printDpi->value());
        m_ui->printHeight->setValue(m_printSize.height() * m_ui->printDpi->value());
    }
    if(index == 1) { //convert to cm
         m_ui->printWidth->setValue(m_printSize.width() * 2.54);
         m_ui->printHeight->setValue(m_printSize.height() * 2.54);
    } else if (index == 2) {
         m_ui->printWidth->setValue(m_printSize.width());
         m_ui->printHeight->setValue(m_printSize.height());
    }
}

void ExportWizard::slotPrintWidthChanged(double newWidth)
{
    switch(m_ui->printUnity->currentIndex()) {
        case 0: // Convert pixels to inches
            m_printSize.setWidth(newWidth/m_ui->printDpi->value());
            break;
        case 1: //Convert cm to inches
            m_printSize.setWidth(newWidth/2.54);
            break;
        case 2:
            m_printSize.setWidth(m_ui->printWidth->value());
            break;
    }
}

void ExportWizard::slotPrintHeightChanged(double newHeight)
{
    switch(m_ui->printUnity->currentIndex()) {
        case 0: // Convert pixels to inches
            m_printSize.setHeight(newHeight/m_ui->printDpi->value());
            break;
        case 1: //Convert cm to inches
            m_printSize.setHeight(newHeight/2.54);
            break;
        case 2:
            m_printSize.setHeight(m_ui->printHeight->value());
            break;
    }
}

void ExportWizard::slotFinished(int code)
{
    if (code == QDialog::Accepted) {
        switch (currentId()) {
            case PageWallpaper: setWallpaper(); break;
            case PageImage: saveImage(); break;
            case PagePosteRazor: startPosterazor(); break;
            case PagePrint: print(); break;
            default:
                qWarning("ExportWizard::slotFinished: unhndled end for page %d", currentId());
                break;
        }
    }
}

void ExportWizard::slotModeButtonClicked()
{
    setPage(sender()->property("nextPageId").toInt());
}

void ExportWizard::slotOpenLink(const QString & address)
{
    QDesktopServices::openUrl(QUrl(address));
}
