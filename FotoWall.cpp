/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "FotoWall.h"
#include "Desk.h"
#include "RenderOpts.h"
#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFile>
#include <QImageReader>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>

// current location and 'check string' for the tutorial
#define TUTORIAL_URL QUrl("http://fosswire.com/post/2008/09/fotowall-make-wallpaper-collages-from-your-photos/")
#define TUTORIAL_STRING "Peter walks you through how to use Fotowall"

class FWGraphicsView : public QGraphicsView {
    public:
        FWGraphicsView(Desk * desk, QWidget * parent)
            : QGraphicsView(desk, parent)
            , m_desk(desk)
        {
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setInteractive(true);
            setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::SmoothPixmapTransform */);
            setDragMode(QGraphicsView::NoDrag);
            setAcceptDrops(true);
            setFrameStyle(QFrame::NoFrame);
            // can't activate the cache mode by default, since it inhibits dynamical background picture changing
            //setCacheMode(CacheBackground);
        }

    protected:
        void resizeEvent(QResizeEvent * /*event*/) {
            m_desk->resize(contentsRect().size());
        }

    private:
        Desk * m_desk;
};


FotoWall::FotoWall(QWidget * parent)
    : QWidget(parent)
    , ui(new Ui::FotoWall())
    , m_view(0)
    , m_desk(0)
{
    // initial 'normal' size
    QRect geom = QApplication::desktop()->availableGeometry();
    resize(2 * geom.width() / 3, 2 * geom.height() / 3);

    // init ui
    ui->setupUi(this);
    ui->tutorialLabel->setVisible(false);
    setWindowIcon( QIcon(":/data/fotowall.png") );

    // create our custom desk
    m_desk = new Desk(this);

    // add the graphicsview
    m_view = new FWGraphicsView(m_desk, ui->centralWidget);
    QVBoxLayout * lay = new QVBoxLayout(ui->centralWidget);
    lay->setAlignment(Qt::AlignHCenter);
    lay->setSpacing(0);
    lay->setMargin(0);
    lay->addWidget(m_view);

    // set the startup project mode
    on_projectType_currentIndexChanged(0);

    // enable the tutorial, if present
    checkForTutorial();
}

FotoWall::~FotoWall()
{
    // dump current layout
    QFile file("autosave.lay");
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        m_desk->save(out);
        file.close();
    }

    // delete everything
    delete m_view;
    delete m_desk;
    delete ui;
}

void FotoWall::showHelp()
{
    m_desk->showHelp();
}

void FotoWall::checkForTutorial()
{
    // hide the tutorial link
    ui->tutorialLabel->setVisible(false);

    // try to get the tutorial page (note, multiple QNAMs will be deleted on app closure)
    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotCheckTutorial(QNetworkReply*)));
    manager->get(QNetworkRequest(TUTORIAL_URL));
}

//BEGIN SizeDialog
#include <QDialog>
#include <QSpinBox>
class SizeDialog : public QDialog {
    public:
        SizeDialog(QWidget * parent = 0)
            : QDialog(parent)
        {
            setWindowTitle(tr("Select Resolution"));

            QLabel * label = new QLabel(tr("The aspect ratio must be kept"), this);
            wSpin = new QSpinBox(this);
            wSpin->setRange(100, 10000);
            hSpin = new QSpinBox(this);
            hSpin->setRange(100, 10000);
            QPushButton * closeButton = new QPushButton(tr("OK"), this);
            connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

            QHBoxLayout * lay = new QHBoxLayout(this);
            lay->addWidget(label);
            lay->addWidget(wSpin);
            lay->addWidget(hSpin);
            lay->addWidget(closeButton);
        }
        QSpinBox * wSpin;
        QSpinBox * hSpin;
};
//END SizeDialog

void FotoWall::saveImage()
{
    QMessageBox::warning(0, tr("Warning"), tr("This function is being rewritten for version 0.6.\nIn the meantime, while not the optimum, you can still get high quality results ;-)"));

    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose the Image file"), QString(), tr("Image (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if (fileName.isNull())
        return;
    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += ".png";

    // get the rendering size
    SizeDialog * sd = new SizeDialog(this);
    sd->wSpin->setValue(m_desk->width());
    sd->hSpin->setValue(m_desk->height());
    if (!sd->exec())
        return;
    int destW = sd->wSpin->value();
    int destH = sd->hSpin->value();
    delete sd;

    // render on the image
    QImage image(destW, destH, QImage::Format_ARGB32);
    image.fill(0);
    QPainter imagePainter(&image);
    imagePainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    m_desk->render(&imagePainter, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    imagePainter.end();

    // save image
    if (!image.save(fileName) || !QFile::exists(fileName)) {
        QMessageBox::warning(this, tr("Rendering Save Error"), tr("Error rendering to this file"));
        return;
    }
    int size = QFile(fileName).size();
    QMessageBox::information(this, tr("Image rendered"), tr("The target image is %1 bytes long").arg(size));
}

void FotoWall::saveCD()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer);
    bool ok = printDialog.exec();
    if(!ok) return;

    QImage image(1410, 1410, QImage::Format_ARGB32);
    image.fill(0);
    QPainter paintimg(&image);
    paintimg.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    // Render on the image
    m_desk->render(&paintimg, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    paintimg.end();

    // And then print
    // dpi resolution for exporting at the right size
    printer.setResolution(300);
    printer.setPaperSize(QPrinter::A4);
    QPainter paint(&printer);
    paint.drawImage(image.rect(), image);
}

void FotoWall::saveDVD()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer);
    bool ok = printDialog.exec();
    if(!ok) return;

    QImage image(3289, 2160, QImage::Format_ARGB32);
    image.fill(0);
    QPainter paintimg(&image);
    paintimg.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    // Render on the image
    m_desk->render(&paintimg, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    paintimg.end();

    QMatrix matrix;
    // Rotate the image (because a DVD cover needs to be print in landscape mode).
    matrix.rotate(90);
    image = image.transformed(matrix);

    // And then print
    // dpi resolution for exporting at the right size
    printer.setResolution(300);
    printer.setPaperSize(QPrinter::A4);
    QPainter paint(&printer);
    paint.drawImage(image.rect(), image);
}

void FotoWall::on_projectType_currentIndexChanged(int index)
{
    int w, h;
    switch (index) {
        case 0:
            //Normal project
            m_view->setMinimumSize(m_view->minimumSizeHint());
            m_view->setMaximumSize(QSize(16777215, 16777215));
            showMaximized();
            ui->exportButton->setText(tr("export..."));
            m_desk->setProjectMode(Desk::ModeNormal);
            break;

        case 1:
            // CD cover
            // A CD cover is a 4.75x4.715 inches square. To get the size in pixel, we must multiply by the dpi (dot per inch)
            w = (int)(4.75 * (float)m_view->logicalDpiX());
            h = (int)(4.75 * (float)m_view->logicalDpiY());
            m_view->setFixedSize(w, h);
            showNormal();
            ui->exportButton->setText(tr("print..."));
            m_desk->setProjectMode(Desk::ModeCD);
            break;

       case 2:
            //DVD cover
            w = (int)(10.83 * (float)m_view->logicalDpiX());
            h = (int)(7.2 * (float)m_view->logicalDpiY());
            m_view->setFixedSize(w, h);
            showNormal();
            ui->exportButton->setText(tr("print..."));
            m_desk->setProjectMode(Desk::ModeDVD);
            break;
    }
}

void FotoWall::on_addPictures_clicked()
{
    // build the extensions list
    QString extensions;
    foreach (const QByteArray & format, QImageReader::supportedImageFormats())
        extensions += "*." + format + " ";

    // show the files dialog
    QStringList fileNames = QFileDialog::getOpenFileNames(ui->centralWidget, tr("Select one or more pictures to add"), QString(), tr("Images (%1)").arg(extensions));
    if (!fileNames.isEmpty())
        m_desk->addPictures(fileNames);
}

void FotoWall::on_addText_clicked()
{
    m_desk->addTextContent();
}

void FotoWall::on_helpLabel_linkActivated(const QString & /*link*/)
{
    m_desk->showHelp();
}

void FotoWall::on_tutorialLabel_linkActivated(const QString & /*link*/)
{
    int answer = QMessageBox::question(this, tr("Opening the Web Tutorial"), tr("The Tutorial is provided on Fosswire by Peter Upfold.\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QDesktopServices::openUrl(TUTORIAL_URL);
}

void FotoWall::on_loadButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Layout file"), QString(), "Layouts (*.lay)");
    if (fileName.isNull())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Error"), tr("Error load layout from this file"));
        return;
    }
    QDataStream in(&file);
    m_desk->restore(in);
}

void FotoWall::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select Layout file"), QString(), "Layouts (*.lay)");
    if (fileName.isNull())
        return;
    if (!fileName.endsWith(".lay", Qt::CaseInsensitive))
        fileName += ".lay";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Save Error"), tr("Error saving layout to this file"));
        return;
    }
    QDataStream out(&file);
    m_desk->save(out);
}

void FotoWall::on_exportButton_clicked()
{
    RenderOpts::HQRendering = true;
    // check to project type for saving
    switch (m_desk->projectMode()) {
        default:
            saveImage();
            break;

        case Desk::ModeCD:
            saveCD();
            break;

        case Desk::ModeDVD:
            saveDVD();
            break;
    }
    RenderOpts::HQRendering = false;
}

void FotoWall::on_quitButton_clicked()
{
    QCoreApplication::quit();
}

void FotoWall::slotCheckTutorial(QNetworkReply * reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;

    QString htmlCode = reply->readAll();
    bool tutorialValid = htmlCode.contains(TUTORIAL_STRING, Qt::CaseInsensitive);
    ui->tutorialLabel->setVisible(tutorialValid);
}
