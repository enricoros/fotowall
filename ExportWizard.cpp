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
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPrinter>
#include <QPrintDialog>
#include <QUrl>

#if defined(Q_OS_WINDOWS)
#include <windows.h>    // for background changing stuff
#endif

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

#if !defined(Q_OS_WINDOWS) && !defined(Q_OS_LINUX)
#warning "Implement background change for this OS"
    m_ui->clWallpaper->hide();
#endif

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

void ExportWizard::setWallpaper()
{
    // find a new filePath
    QString filePath;
    int fileNumber = 0;
    while (filePath.isEmpty() || QFile::exists(filePath))
        filePath = QDir::homePath() + QDir::separator() + "fotowall-background" + QString::number(++fileNumber) + ".jpeg";

    // create and image into the home dir
    QRect deskGeometry = QApplication::desktop()->screenGeometry();
    QImage image = m_desk->renderedImage(deskGeometry.size());
    if (!image.save(filePath, "JPG", 95)) {
        QMessageBox::warning(this, tr("Wallpaper Error"), tr("Can't save the image to disk."));
        return;
    }

#if defined(Q_OS_WINDOWS)
    //Set new background path
    QSettings appSettings("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    appSettings.setValue("Wallpaper", filePath);
    QByteArray ba = filePath.toLatin1();

    //Notification to windows refresh desktop
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#elif defined(Q_OS_LINUX)
    // KDE4
    if (QString(qgetenv("KDE_SESSION_VERSION")).startsWith("4"))
        QMessageBox::warning(this, tr("Manual Wallpaper Change"), tr("KDE4 doesn't yet support changing wallpaper automatically.\nGo to the Desktop Settings and select the file:\n  %1").arg(filePath));

    // KDE3
    QString kde3cmd = "dcop kdesktop KBackgroundIface setWallpaper '" + filePath + "' 6";
    QProcess::startDetached(kde3cmd);

    // Gnome2
    QString gnome2Cmd = "gconftool -t string -s /desktop/gnome/background/picture_filename '" + filePath + "'";
    QProcess::startDetached(gnome2Cmd);
#else
#warning "Implement background change for this OS"
#endif
}

void ExportWizard::saveImage()
{
    if (m_ui->filePath->text().isEmpty()) {
        QMessageBox::warning(this, tr("No file selected !"), tr("You need to choose a file path for saving."));
        return;
    }
    QString fileName = m_ui->filePath->text();

    // get the rendering size
    int destW = m_ui->saveWidth->value();
    int destH = m_ui->saveHeight->value();

    // render on the image
    QImage image = m_desk->renderedImage(QSize(destW, destH));

    // rotate image if requested
    if (m_ui->saveLandscape->isChecked()) {
        // Save in landscape mode, so rotate
        QMatrix matrix;
        matrix.rotate(90);
        image = image.transformed(matrix);
    }

    // save image
    if (image.save(fileName) && QFile::exists(fileName)) {
        int size = QFileInfo(fileName).size();
        QMessageBox::information(this, tr("Done"), tr("The target image is %1 bytes long").arg(size));
    } else
        QMessageBox::warning(this, tr("Rendering Error"), tr("Error rendering to the file '%1'").arg(fileName));
}

#include <math.h>
#include "posterazorcore.h"
#include "controller.h"
#include "wizard.h"
#include "imageloaderqt.h"
void ExportWizard::startPosterazor()
{
    static const quint32 posterPixels = 6 * 1000000; // Megapixels * 3 bytes!
    // We will use up the whole posterPixels for the render, respecting the aspect ratio.
    const qreal widthToHeightRatio = m_desk->width() / m_desk->height();
    // Thanks to colleague Oswald for some of the math :)
    const int posterPixelWidth = int(sqrt(widthToHeightRatio * posterPixels));
    const int posterPixelHeight = posterPixels / posterPixelWidth;

    static const QLatin1String settingsGroup("posterazor");
    QSettings settings;
    settings.beginGroup(settingsGroup);

    // TODO: Eliminate Poster size in %
    ImageLoaderQt loader;
    loader.setQImage(m_desk->renderedImage(QSize(posterPixelWidth, posterPixelHeight)));
    PosteRazorCore posterazor(&loader);
    posterazor.readSettings(&settings);
    Wizard *wizard = new Wizard;
    Controller controller(&posterazor, wizard);
    controller.setImageLoadingAvailable(false);
    controller.setPosterSizeModeAvailable(Types::PosterSizeModePercentual, false);
#if QT_VERSION >= 0x040500
    QDialog dialog(this, Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
#else
    QDialog dialog(this, Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint);
#endif
	dialog.setWindowTitle(tr("Export poster"));
    dialog.setLayout(new QVBoxLayout);
    dialog.layout()->addWidget(wizard);
    dialog.resize(640, 480);
    dialog.exec();
    settings.sync();
    posterazor.writeSettings(&settings);
}

void ExportWizard::print()
{
    int dpi = m_ui->printDpi->value();
    float w = m_ui->printWidth->value(), h=m_ui->printHeight->value();
    if (m_ui->printUnity->currentIndex() == 0) { // If pixels
        m_printSize.setWidth(w/dpi);
        m_printSize.setHeight(h/dpi);
    } else if (m_ui->printUnity->currentIndex() == 1) { // If cm
        m_printSize.setWidth(w/2.54); //Convert in inches
        m_printSize.setHeight(h/2.54);
    } else if (m_ui->printUnity->currentIndex() == 2) { //If inches
        m_printSize.setWidth(w);
        m_printSize.setHeight(h);
    }

    int width = (int)(m_printSize.width() * (float)dpi);
    int height = (int)(m_printSize.height() * (float)dpi);
    m_desk->printAsImage(dpi, QSize(width, height), m_ui->printLandscape->isChecked());
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
