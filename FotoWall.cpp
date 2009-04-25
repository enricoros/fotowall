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
#include "OcsConnector.h"
#include "RenderOpts.h"
#include "VideoProvider.h"
#include "ui_FotoWall.h"
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFile>
#include <QImageReader>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPrinter>
#include <QPrintDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include "ModeInfo.h"
#include "ExactSizeDialog.h"

// current location and 'check string' for the tutorial
#define TUTORIAL_URL QUrl("http://fosswire.com/post/2008/09/fotowall-make-wallpaper-collages-from-your-photos/")
#define TUTORIAL_STRING "Peter walks you through how to use Foto"

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
            // FIXME: enable the RubberBandDrag. I get rendering artifacts (Enrico)
            setDragMode(QGraphicsView::NoDrag);
            setAcceptDrops(true);
            setFrameStyle(QFrame::NoFrame);
            // can't activate the cache mode by default, since it inhibits dynamical background picture changing
            //setCacheMode(CacheBackground);
        }

    protected:
        void resizeEvent(QResizeEvent * event)
        {
            m_desk->resize(contentsRect().size());
            QGraphicsView::resizeEvent(event);
        }

    private:
        Desk * m_desk;
};

FotoWall::FotoWall(QWidget * parent)
    : QWidget(parent)
    , ui(new Ui::FotoWall())
    , m_view(0)
    , m_desk(0)
    , m_ocsConnector(0)
{
    // initial 'normal' size
    QRect geom = QApplication::desktop()->availableGeometry();
    resize(2 * geom.width() / 3, 2 * geom.height() / 3);

    // create our custom desk
    m_desk = new Desk(this);

    // init ui
    ui->setupUi(this);

    // set the decoration menu
    ui->decoButton->setMenu(createDecorationMenu());
    ui->helpButton->setMenu(createHelpMenu());

    ///ui->tutorialLabel->setVisible(false);

    ui->addMirror->setVisible(VideoProvider::instance()->inputCount() > 0);
    connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVideoInputsChanged(int)));

    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());
    setWindowIcon(QIcon(":/data/fotowall.png"));


    // add the graphicsview
    m_view = new FWGraphicsView(m_desk, ui->centralWidget);
    QVBoxLayout * lay = new QVBoxLayout(ui->centralWidget);
    lay->setAlignment(Qt::AlignHCenter);
    lay->setSpacing(0);
    lay->setMargin(0);
    lay->addWidget(m_view);
    m_view->setFocus();

    // create gui actions
    createActions();

    // set the startup project mode
    on_projectType_currentIndexChanged(0);
    m_modeInfo.setDeskDpi(m_view->logicalDpiX(), m_view->logicalDpiY());
    m_modeInfo.setPrintDpi(300);

    // enable the tutorial, if present
    checkForTutorial();

    // TEMP HACK FIXME
    m_ocsConnector = new OcsConnector(this);
    connect(m_ocsConnector, SIGNAL(kbListed(const KnowledgeItemV1List &)), this, SLOT(slotOcsKbItems(const KnowledgeItemV1List &)));
    m_ocsConnector->setServiceUrl(QUrl("http://api.opendesktop.org"));
    m_ocsConnector->setApiKey("API4969719359710210465243692945282815516785946916238413642488886219");
    m_ocsConnector->kbList("71320");
}

FotoWall::~FotoWall()
{
    // dump current layout
    m_desk->save(QDir::tempPath() + QDir::separator() + "autosave.fotowall", this);

    // delete everything
    delete m_view;
    delete m_desk;
    delete ui;
}

void FotoWall::showIntroduction()
{
    m_desk->showIntroduction();
}

void FotoWall::checkForTutorial()
{
    // hide the tutorial link
    ///ui->tutorialLabel->setVisible(false);

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

    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose the Image file"), QString(), tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
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
    m_desk->renderVisible(&imagePainter, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    imagePainter.end();

    // save image
    if (!image.save(fileName) || !QFile::exists(fileName)) {
        QMessageBox::warning(this, tr("Rendering Error"), tr("Error rendering to the file '%1'").arg(fileName));
        return;
    }
    int size = QFile(fileName).size();
    QMessageBox::information(this, tr("Done"), tr("The target image is %1 bytes long").arg(size));
}

void FotoWall::saveExactSize()
{
    QPrinter printer;
    QPrintDialog printDialog(&printer);
    bool ok = printDialog.exec();
    if(!ok) return;

    QImage image(m_modeInfo.printPixelSize(), QImage::Format_ARGB32);
    image.fill(0);
    QPainter paintimg(&image);
    paintimg.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    // Render on the image
    m_desk->renderVisible(&paintimg, image.rect(), m_desk->sceneRect(), Qt::KeepAspectRatio);
    paintimg.end();

    if(m_modeInfo.landscape()) {
        // Print in landscape mode, so rotate
        QMatrix matrix;
        matrix.rotate(90);
        image = image.transformed(matrix);
    }

    // And then print
    // dpi resolution for exporting at the right size
    printer.setResolution(m_modeInfo.printDpi());
    printer.setPaperSize(QPrinter::A4);
    QPainter paint(&printer);
    paint.drawImage(image.rect(), image);
}

void FotoWall::createActions()
{
    // select all
    QAction * aSA = new QAction(tr("Select all"), this);
    aSA->setShortcut(tr("CTRL+A"));
    connect(aSA, SIGNAL(triggered()), this, SLOT(slotActionSelectAll()));
    addAction(aSA);
}

QMenu * FotoWall::createDecorationMenu()
{
    QMenu * menu = new QMenu();

    QAction * aTop = new QAction(tr("Top bar"), this);
    aTop->setCheckable(true);
    aTop->setChecked(m_desk->topBarEnabled());
    connect(aTop, SIGNAL(toggled(bool)), this, SLOT(slotDecoTopBar(bool)));
    menu->addAction(aTop);

    QAction * aBottom = new QAction(tr("Bottom bar"), this);
    aBottom->setCheckable(true);
    aBottom->setChecked(m_desk->bottomBarEnabled());
    connect(aBottom, SIGNAL(toggled(bool)), this, SLOT(slotDecoBottomBar(bool)));
    menu->addAction(aBottom);

    menu->addSeparator();

    QAction * aSetTitle = new QAction(tr("Set title..."), this);
    aSetTitle->setShortcut(tr("CTRL+T"));
    connect(aSetTitle, SIGNAL(triggered()), this, SLOT(slotDecoSetTitle()));
    menu->addAction(aSetTitle);

    QAction * aClearTitle = new QAction(tr("Clear title"), this);
    connect(aClearTitle, SIGNAL(triggered()), this, SLOT(slotDecoClearTitle()));
    menu->addAction(aClearTitle);

    return menu;
}

QMenu * FotoWall::createHelpMenu()
{
    QMenu * menu = new QMenu();

    // FIXME: make this public
    QAction * aIntroduction = new QAction(tr("Introduction"), this);
    connect(aIntroduction, SIGNAL(triggered()), this, SLOT(slotHelpIntroduction()));
    menu->addAction(aIntroduction);

    QAction * aTutorial = new QAction(tr("Tutorial"), this);
    connect(aTutorial, SIGNAL(triggered()), this, SLOT(slotHelpTutorial()));
    menu->addAction(aTutorial);

    QAction * aSupport = new QAction(tr("Support (%1)").arg(0) /*FIXME*/, this);
    connect(aSupport, SIGNAL(triggered()), this, SLOT(slotHelpSupport()));
    menu->addAction(aSupport);

    return menu;
}

void FotoWall::loadNormalProject()
{
    m_modeInfo.setRealSizeInches(-1,-1); // Unset the size (for the saving function)
    static bool skipFirstMaximizeHack = true;
    m_view->setMinimumSize(m_view->minimumSizeHint());
    m_view->setMaximumSize(QSize(16777215, 16777215));
    if (skipFirstMaximizeHack)
        skipFirstMaximizeHack = false;
    else
        showMaximized();
    ui->exportButton->setText(tr("export..."));
    m_desk->setProjectMode(Desk::ModeNormal);
    ui->projectType->setCurrentIndex(0);
}
void FotoWall::loadCDProject()
{
    // A CD cover is a 4.75x4.715 inches square.
    m_modeInfo.setRealSizeInches(4.75, 4.75);
    m_modeInfo.setLandscape(false);
    m_view->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print..."));
    m_desk->setProjectMode(Desk::ModeCD);
    ui->projectType->setCurrentIndex(1);
}
void FotoWall::loadDVDProject()
{
    m_modeInfo.setRealSizeInches(10.83, 7.2);
    m_modeInfo.setLandscape(true);
    m_view->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print..."));
    m_desk->setProjectMode(Desk::ModeDVD);
    ui->projectType->setCurrentIndex(2);
}
void FotoWall::loadExactSizeProject()
{
    // Exact size mode
    if(m_modeInfo.realSize().isEmpty()) {
        ExactSizeDialog sizeDialog;
        if(sizeDialog.exec() != QDialog::Accepted) {
            return;
        }
        float w = sizeDialog.ui.widthSpinBox->value();
        float h = sizeDialog.ui.heightSpinBox->value();
        int dpi = sizeDialog.ui.dpiSpinBox->value();
        bool landscape = sizeDialog.ui.landscapeCheckBox->isChecked();
        m_modeInfo.setLandscape(landscape);
        m_modeInfo.setPrintDpi(dpi);
        if(sizeDialog.ui.unityComboBox->currentIndex() == 0)
            m_modeInfo.setRealSizeCm(w, h);
        else
            m_modeInfo.setRealSizeInches(w, h);
    }
    m_view->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print..."));
    m_desk->setProjectMode(Desk::ModeExactSize);
    ui->projectType->setCurrentIndex(3);
}
void FotoWall::on_projectType_currentIndexChanged(int index)
{
    m_modeInfo.setRealSizeInches(-1,-1); // Unset the size (so if it is a mode that require
                                        // asking size, it will be asked !
    switch (index) {
        case 0:
            //Normal project
            loadNormalProject();
            break;

        case 1:
            // CD cover
            loadCDProject();
            break;

        case 2:
            //DVD cover
            loadDVDProject();
            break;
        case 3:
            loadExactSizeProject();
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

void FotoWall::on_addMirror_clicked()
{
    m_desk->addVideoContent(0);
}

void FotoWall::on_loadButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select FotoWall file"), QString(), tr("FotoWall (*.fotowall)"));
    load(fileName);
}

void FotoWall::load(QString &fileName)
{
    if (!fileName.isNull())
        m_desk->restore(fileName, this);
}

ModeInfo FotoWall::getModeInfo()
{
    return m_modeInfo;
}
void FotoWall::setModeInfo(ModeInfo modeInfo)
{
    m_modeInfo = modeInfo;
    m_modeInfo.setDeskDpi(m_view->logicalDpiX(), m_view->logicalDpiY());
}
void FotoWall::restoreMode(int mode)
{
    on_projectType_currentIndexChanged(mode);
}

void FotoWall::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select FotoWall file"), QString(), "FotoWall (*.fotowall)");
    if (fileName.isNull())
        return;
    if (!fileName.endsWith(".fotowall", Qt::CaseInsensitive))
        fileName += ".fotowall";

    m_desk->save(fileName, this);
}

void FotoWall::on_exportButton_clicked()
{
    RenderOpts::HQRendering = true;
    // check to project type for saving
    switch (m_desk->projectMode()) {
        case Desk::ModeNormal:
            saveImage();
            break;

        default:
            saveExactSize();
            break;
    }
    RenderOpts::HQRendering = false;
}

void FotoWall::on_quitButton_clicked()
{
    QCoreApplication::quit();
}

void FotoWall::slotDecoTopBar(bool checked)
{
    m_desk->setTopBarEnabled(checked);
}

void FotoWall::slotDecoBottomBar(bool checked)
{
    m_desk->setBottomBarEnabled(checked);
}

void FotoWall::slotDecoSetTitle()
{
    // set a dummy title, if none
    if (m_desk->titleText().isEmpty())
        m_desk->setTitleText("...");

    // change title dialog
    bool ok = false;
    QString title = QInputDialog::getText(0, tr("Title"), tr("Insert the title"), QLineEdit::Normal, m_desk->titleText(), &ok);
    if (ok)
        m_desk->setTitleText(title);
}

void FotoWall::slotDecoClearTitle()
{
    m_desk->setTitleText(QString());
}

void FotoWall::slotHelpIntroduction()
{
    m_desk->showIntroduction();
}

void FotoWall::slotHelpTutorial()
{
    int answer = QMessageBox::question(this, tr("Opening the Web Tutorial"), tr("The Tutorial is provided on Fosswire by Peter Upfold.\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QDesktopServices::openUrl(TUTORIAL_URL);
}

void FotoWall::slotHelpSupport()
{
}

void FotoWall::slotActionSelectAll()
{
    m_desk->selectAllContent();
}

void FotoWall::slotCheckTutorial(QNetworkReply * reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;

    QString htmlCode = reply->readAll();
    bool tutorialValid = htmlCode.contains(TUTORIAL_STRING, Qt::CaseInsensitive);
    ///ui->tutorialLabel->setVisible(tutorialValid);
}

#include <QDebug>
void FotoWall::slotOcsKbItems(const KnowledgeItemV1List & items)
{
    qWarning("FotoWall::slotOcsKbItems: got %d items", items.size());
    foreach (KnowledgeItemV1 * item, items) {
        qWarning() << item->name() << item->description() << item->answer();
    }
}

void FotoWall::slotVideoInputsChanged(int count)
{
    // maybe blink or something?
    ui->addMirror->setVisible(count > 0);
}
