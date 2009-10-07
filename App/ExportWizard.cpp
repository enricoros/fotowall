/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
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

#include "Canvas/Canvas.h"
#include "App.h"
#include "Settings.h"
#include "ui_ExportWizard.h"
#include "controller.h"
#include "imageloaderqt.h"
#include "posterazorcore.h"
#include "wizard.h"

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
#include <QSettings>
#include <QSvgGenerator>
#include <QUrl>
#include <math.h>

#if defined(Q_OS_WIN)
#include <windows.h>    // for background changing stuff
#endif

#define POSTERAZOR_WEBSITE_LINK "http://posterazor.sourceforge.net/"
#define POSTERAZOR_TUTORIAL_LINK "http://www.youtube.com/watch?v=p7XsFZ4Leo8"

ExportWizard::ExportWizard(Canvas * canvas)
    : QWizard()
    , m_ui(new Ui::ExportWizard)
    , m_canvas(canvas)
    , m_nextId(0)
{
    // create and init UI
    m_ui->setupUi(this);
    connect(m_ui->clWallpaper, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clImage, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clPosteRazor, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clPrint, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    connect(m_ui->clSvg, SIGNAL(clicked()), this, SLOT(slotModeButtonClicked()));
    m_ui->prWebLabel->setText("<html><body><a href='" POSTERAZOR_WEBSITE_LINK "'>" + m_ui->prWebLabel->text() + "</a></body></html>" );
    connect(m_ui->prWebLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(slotOpenLink(const QString &)));
    m_ui->prTutorialLabel->setText("<html><body><a href='" POSTERAZOR_TUTORIAL_LINK "'>" + m_ui->prTutorialLabel->text() + "</a></body></html>" );
    connect(m_ui->prTutorialLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(slotOpenLink(const QString &)));

#if !defined(Q_OS_WIN) && !defined(Q_OS_LINUX)
#warning "Implement background change for this OS"
    m_ui->clWallpaper->hide();
#endif

    // set default sizes
    m_ui->saveHeight->setValue(m_canvas->height());
    m_ui->saveWidth->setValue(m_canvas->width());
    m_ui->printWidth->setValue(m_canvas->width());
    m_ui->printHeight->setValue(m_canvas->height());
    m_printSize.setWidth(m_canvas->width()/m_ui->printDpi->value());
    m_printSize.setHeight(m_canvas->height()/m_ui->printDpi->value());

    // connect buttons
    connect(m_ui->chooseFilePath, SIGNAL(clicked()), this, SLOT(slotChoosePath()));
    connect(m_ui->printUnity, SIGNAL(currentIndexChanged(int)), this, SLOT(slotPrintUnityChanged(int)));
    connect(m_ui->printWidth, SIGNAL(valueChanged(double)), this, SLOT(slotPrintWidthChanged(double)));
    connect(m_ui->printHeight, SIGNAL(valueChanged(double)), this, SLOT(slotPrintHeightChanged(double)));
    connect(m_ui->svgChooseFilePath, SIGNAL(clicked()), this, SLOT(slotChooseSvgPath()));

    // configure Wizard
    setOptions(NoDefaultButton | NoBackButtonOnStartPage | IndependentPages);
    setPage(PageMode);
    setMinimumWidth(400);

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
#if defined(Q_OS_WIN)
        filePath = QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + "fotowall-background" + QString::number(++fileNumber) + ".bmp";
#else
        filePath = QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + "fotowall-background" + QString::number(++fileNumber) + ".jpg";
#endif

    // render the image
    QImage image;
    QSize sceneSize(m_canvas->width(), m_canvas->height());
    QSize desktopSize = QApplication::desktop()->screenGeometry().size();
    if (m_ui->wbZoom->isChecked())
        image = m_canvas->renderedImage(desktopSize, Qt::KeepAspectRatioByExpanding);
    else if (m_ui->wbScaleKeep->isChecked())
        image = m_canvas->renderedImage(desktopSize, Qt::KeepAspectRatio);
    else if (m_ui->wbScaleIgnore->isChecked())
        image = m_canvas->renderedImage(desktopSize, Qt::IgnoreAspectRatio);
    else
        image = m_canvas->renderedImage(sceneSize);

    // save the right kind of image into the home dir
#if defined(Q_OS_WIN)
    if (!image.save(filePath, "BMP")) {
#else
    if (!image.save(filePath, "JPG", 100)) {
#endif
        QMessageBox::warning(this, tr("Wallpaper Error"), tr("Can't save the image to disk."));
        return;
    }

#if defined(Q_OS_WIN)
    //Set new background path
    {QSettings appSettings("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    appSettings.setValue("ConvertedWallpaper", filePath);
    appSettings.setValue("Wallpaper", filePath);}

    //Notification to windows refresh desktop
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, true, (void*)qPrintable(filePath), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#elif defined(Q_OS_LINUX)
    // KDE4
    if (QString(qgetenv("KDE_SESSION_VERSION")).startsWith("4"))
        QMessageBox::warning(this, tr("Manual Wallpaper Change"), tr("KDE4 doesn't yet support changing wallpaper automatically.\nGo to the Desktop Settings and select the file:\n  %1").arg(filePath));

    // KDE3
    QString kde3cmd = "dcop kdesktop KBackgroundIface setWallpaper '" + filePath + "' 6";
    QProcess::startDetached(kde3cmd);

    // Gnome2
    QString gnome2Cmd = "gconftool -t string -s /desktop/gnome/background/picture_filename " + filePath ;
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
    QSize imageSize(m_ui->saveWidth->value(), m_ui->saveHeight->value());

    // render the image
    QImage image;
    bool hideTools = !m_ui->imgAsIsBox->isChecked();
    if (m_ui->ibZoom->isChecked())
        image = m_canvas->renderedImage(imageSize, Qt::KeepAspectRatioByExpanding, hideTools);
    else if (m_ui->ibScaleKeep->isChecked())
        image = m_canvas->renderedImage(imageSize, Qt::KeepAspectRatio, hideTools);
    else
        image = m_canvas->renderedImage(imageSize, Qt::IgnoreAspectRatio, hideTools);

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

void ExportWizard::startPosterazor()
{
    static const quint32 posterPixels = 6 * 1000000; // Megapixels * 3 bytes!
    // We will use up the whole posterPixels for the render, respecting the aspect ratio.
    const qreal widthToHeightRatio = m_canvas->width() / m_canvas->height();
    // Thanks to colleague Oswald for some of the math :)
    const int posterPixelWidth = int(sqrt(widthToHeightRatio * posterPixels));
    const int posterPixelHeight = posterPixels / posterPixelWidth;

    static const QLatin1String settingsGroup("posterazor");
    App::settings->beginGroup(settingsGroup);

    // TODO: Eliminate Poster size in %
    ImageLoaderQt loader;
    loader.setQImage(m_canvas->renderedImage(QSize(posterPixelWidth, posterPixelHeight)));
    PosteRazorCore posterazor(&loader);
    posterazor.readSettings(App::settings);
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
    App::settings->sync();
    posterazor.writeSettings(App::settings);
    App::settings->endGroup();
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
    Qt::AspectRatioMode ratioMode = m_ui->printKeepRatio->isChecked() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio;
    m_canvas->printAsImage(dpi, QSize(width, height), m_ui->printLandscape->isChecked(), ratioMode);
}

void ExportWizard::saveSvg()
{
    if (m_ui->svgFilePath->text().isEmpty()) {
        QMessageBox::warning(this, tr("No file selected !"), tr("You need to choose a file path for saving."));
        return;
    }
    QString svgFileName = m_ui->svgFilePath->text();

    // get the rendering size
    QRect svgRect(m_canvas->sceneRect().toRect());

    // create the SVG writer
    QSvgGenerator generator;
    generator.setFileName(svgFileName);
    generator.setSize(svgRect.size());
#if QT_VERSION >= 0x040500
    generator.setResolution(physicalDpiX());
    generator.setViewBox(svgRect);
    generator.setTitle(m_canvas->titleText());
    generator.setDescription(tr("Created with %1").arg(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion()));
#endif

    // paint over the writer
    QPainter painter(&generator);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
    m_canvas->renderVisible(&painter, svgRect, svgRect, Qt::IgnoreAspectRatio, !m_ui->svgAsIsBox->isChecked());
    painter.end();
}

void ExportWizard::setPage(int pageId)
{
    // adapt buttons
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::BackButton;
    if (pageId >= PageWallpaper && pageId <= PageSvg)
        layout << QWizard::FinishButton;
    layout << QWizard::CancelButton;
    setButtonLayout(layout);

    // change page
    m_nextId = pageId;
    next();

    // execute on-entry code
    switch (pageId) {
        case PageImage:
            if (m_ui->filePath->text().isEmpty())
                slotChoosePath();
            break;
        case PageSvg:
            if (m_ui->svgFilePath->text().isEmpty())
                slotChooseSvgPath();
            break;
    }
}

int ExportWizard::nextId() const
{
    // dynamic page ordering
    const int pageId = currentId();

    // mode selection: return the id of the next page (set by the linkbuttons)
    if (pageId == PageMode)
        return m_nextId;

    // final pages
    if (pageId >= PageWallpaper && pageId <= PageSvg)
        return -1;

    // fallback
    qWarning("ExportWizard::nextId: unhandled nextId for page %d", pageId);
    return -1;
}

static QString getSavePath(const QString & initialValue, const QString & defaultExt, const QString & title, const QString & type)
{
    // make up the default save path (stored as 'Fotowall/ExportDir')
    QString defaultSavePath = initialValue;
    if (defaultSavePath.isEmpty()) {
        defaultSavePath = ExportWizard::tr("Unnamed %1.%2").arg(QDate::currentDate().toString()).arg(defaultExt);
        if (App::settings->contains("Fotowall/ExportDir"))
            defaultSavePath.prepend(App::settings->value("Fotowall/ExportDir").toString() + QDir::separator());
    }

    // ask the file name, validate it, store back to settings
    QString fileName = QFileDialog::getSaveFileName(0, title, defaultSavePath, type);
    if (!fileName.isEmpty()) {
        App::settings->setValue("Fotowall/ExportDir", QFileInfo(fileName).absolutePath());
        if (QFileInfo(fileName).suffix().isEmpty())
            fileName += "." + defaultExt;
    }
    return fileName;
}

void ExportWizard::slotChoosePath()
{
    QString savePath = getSavePath(m_ui->filePath->text(), "png",
                                   tr("Choose the Image file"),
                                   tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if (!savePath.isEmpty())
        m_ui->filePath->setText(savePath);
}

void ExportWizard::slotChooseSvgPath()
{
    QString savePath = getSavePath(m_ui->svgFilePath->text(), "svg",
                                   tr("Choose the SVG file"),
                                   tr("SVG (*.svg)"));
    if (!savePath.isEmpty())
        m_ui->svgFilePath->setText(savePath);
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
            case PageSvg: saveSvg(); break;
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
