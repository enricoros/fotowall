/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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

#include "MainWindow.h"
#include "Desk.h"
#include "ExactSizeDialog.h"
#include "ExportWizard.h"
#include "ModeInfo.h"
#include "RenderOpts.h"
#include "VersionCheckDialog.h"
#include "VideoProvider.h"
#include "WarningBox.h"
#include "XmlRead.h"
#include "XmlSave.h"
#include <QAction>
#include <QApplication>
#include <QDir>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFile>
#include <QImageReader>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

// current location and 'check string' for the tutorial
#define TUTORIAL_URL QUrl("http://fosswire.com/post/2008/09/fotowall-make-wallpaper-collages-from-your-photos/")
#define TUTORIAL_STRING "Peter walks you through how to use Foto"
#define ENRICOBLOG_URL QUrl("http://enricoros.wordpress.com/tag/fotowall/")

#include <QCommonStyle>
class RubberBandStyle : public QCommonStyle {
    public:
        void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const
        {
            if (element == CE_RubberBand) {
                painter->save();
                QColor color = option->palette.color(QPalette::Highlight);
                painter->setPen(color);
                color.setAlpha(80); painter->setBrush(color);
                painter->drawRect(option->rect.adjusted(0,0,-1,-1));
                painter->restore();
                return;
            }
            return QCommonStyle::drawControl(element, option, painter, widget);
        }
        int styleHint(StyleHint hint, const QStyleOption * option, const QWidget * widget, QStyleHintReturn * returnData) const
        {
            if (hint == SH_RubberBand_Mask)
                return false;
            return QCommonStyle::styleHint(hint, option, widget, returnData);
        }
};

#include <QGraphicsView>
class FWGraphicsView : public QGraphicsView {
    public:
        FWGraphicsView(QWidget * parent)
            : QGraphicsView(parent)
            , m_desk(0)
        {
            // customize widget
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            setInteractive(true);
            setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::SmoothPixmapTransform */);
            setDragMode(QGraphicsView::RubberBandDrag);
            setAcceptDrops(true);
            setFrameStyle(QFrame::NoFrame);

            // don't autofill the view with the Base brush
            QPalette pal;
            pal.setBrush(QPalette::Base, Qt::NoBrush);
            setPalette(pal);

            // use own style for drawing the RubberBand (opened on the viewport)
            viewport()->setStyle(new RubberBandStyle);

            // can't activate the cache mode by default, since it inhibits dynamical background picture changing
            //setCacheMode(CacheBackground);
        }

        void setDesk(Desk * desk)
        {
            setScene(desk);
            m_desk = desk;
        }

    protected:
        void resizeEvent(QResizeEvent * event)
        {
            if (m_desk)
                m_desk->resize(contentsRect().size());
            QGraphicsView::resizeEvent(event);
        }

    private:
        Desk * m_desk;
};

// added here because it needs the FWGraphicsView declaration
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget * parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow())
    , m_desk(0)
    , m_aHelpTutorial(0)
    , m_aHelpSupport(0)
    , m_gBackActions(0)
    , m_gBackRatioActions(0)
{
    // setup widget
    QRect geom = QApplication::desktop()->availableGeometry();
    resize(2 * geom.width() / 3, 2 * geom.height() / 3);
#if QT_VERSION >= 0x040500
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());
#else
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion() + "   -Limited Edition (Qt 4.4)-");
#endif
    setWindowIcon(QIcon(":/data/fotowall.png"));

    // create our custom desk
    m_desk = new Desk(this);
    connect(m_desk, SIGNAL(backModeChanged()), this, SLOT(slotBackModeChanged()));
    connect(m_desk, SIGNAL(showPropertiesWidget(QWidget*)), this, SLOT(slotShowPropertiesWidget(QWidget*)));

    // init ui
    ui->setupUi(this);
    ui->canvas->setDesk(m_desk);
    ui->canvas->setFocus();
    ui->b1->setDefaultAction(ui->aAddPicture);
    ui->b2->setDefaultAction(ui->aAddText);
    ui->b3->setDefaultAction(ui->aAddWebcam);
    ui->b4->setDefaultAction(ui->aAddFlickr);
#if QT_VERSION >= 0x040500
#ifdef QT_OPENGL_LIB
    ui->accelBox->setEnabled(true);
#endif
    ui->transpBox->setEnabled(true);
#endif
    ui->widgetProperties->collapse();
    ui->widgetCanvas->expand();

    // attach menus
    ///ui->arrangeButton->setMenu(createArrangeMenu());
    ui->backButton->setMenu(createBackgroundMenu());
    ui->decoButton->setMenu(createDecorationMenu());
    ui->onlineHelpButton->setMenu(createOnlineHelpMenu());

    // react to VideoProvider
    ui->aAddWebcam->setVisible(VideoProvider::instance()->inputCount() > 0);
    connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVerifyVideoInputs(int)));

    // create misc actions
    createMiscActions();

    // set the startup project mode
    on_projectType_activated(0);
    m_modeInfo.setDeskDpi(ui->canvas->logicalDpiX(), ui->canvas->logicalDpiY());
    m_modeInfo.setPrintDpi(300);

    // check stuff on the net
    checkForTutorial();
    checkForSupport();
    checkForUpdates();
}

MainWindow::~MainWindow()
{
    // dump current layout
    saveXml(QDir::tempPath() + QDir::separator() + "autosave.fotowall");

    // delete everything
    delete m_desk;
    delete ui;
}

void MainWindow::setModeInfo(ModeInfo modeInfo)
{
    m_modeInfo = modeInfo;
    m_modeInfo.setDeskDpi(ui->canvas->logicalDpiX(), ui->canvas->logicalDpiY());
}

ModeInfo MainWindow::getModeInfo()
{
    return m_modeInfo;
}

void MainWindow::restoreMode(int mode)
{
    if (mode == 3) { // If exact size project
        // Called here not to have the unneeded size dialog
        setExactSizeProject();
    } else {
        on_projectType_activated(mode);
    }
}

void MainWindow::loadXml(const QString & filePath)
{
    if (filePath.isNull())
        return;
    XmlRead *xmlRead = 0;
    try {
        xmlRead = new XmlRead(filePath);
    } catch (...) {
        // If loading failed
        return;
    }
    xmlRead->readProject(this);
    xmlRead->readDesk(m_desk);
    xmlRead->readContent(m_desk);
    delete xmlRead;
}

void MainWindow::saveXml(const QString & filePath) const
{
    XmlSave *xmlSave = 0;
    try {
        xmlSave = new XmlSave(filePath);
    } catch (...) {
        //if saving failled
        return;
    }
    xmlSave->saveProject(m_desk->projectMode(), m_modeInfo);
    xmlSave->saveDesk(m_desk);
    xmlSave->saveContent(m_desk);
    delete xmlSave;
}

void MainWindow::showIntroduction()
{
    m_desk->showIntroduction();
}

void MainWindow::loadImages(QStringList &imagesPath)
{
    m_desk->addPictures(imagesPath);
}

QMenu * MainWindow::createArrangeMenu()
{
    QMenu * menu = new QMenu();

    QAction * aForceField = new QAction(tr("Enable force field"), menu);
    aForceField->setCheckable(true);
    aForceField->setChecked(m_desk->forceFieldEnabled());
    connect(aForceField, SIGNAL(toggled(bool)), this, SLOT(slotArrangeForceField(bool)));
    menu->addAction(aForceField);

    QAction * aNP = new QAction(tr("Auto-arrange new pictures"), menu);
    aNP->setCheckable(true);
    aNP->setChecked(false);
    //connect(aNP, SIGNAL(toggled(bool)), this, SLOT(slotArrangeNew(bool)));
    menu->addAction(aNP);

    menu->addSeparator()->setText(tr("Rearrange"));

    QAction * aAU = new QAction(tr("Uniform"), menu);
    aAU->setEnabled(false);
    //connect(aAU, SIGNAL(triggered()), this, SLOT(slotArrangeUniform()));
    menu->addAction(aAU);

    QAction * aAS = new QAction(tr("Shaped"), menu);
    aAS->setEnabled(false);
    //connect(aAS, SIGNAL(triggered()), this, SLOT(slotArrangeShape()));
    menu->addAction(aAS);

    QAction * aAC = new QAction(tr("Collage"), menu);
    aAC->setEnabled(false);
    //connect(aAC, SIGNAL(triggered()), this, SLOT(slotArrangeCollage()));
    menu->addAction(aAC);

    return menu;
}

QMenu * MainWindow::createBackgroundMenu()
{
    QMenu * menu = new QMenu();
    m_gBackActions = new QActionGroup(menu);
    connect(m_gBackActions, SIGNAL(triggered(QAction*)), this, SLOT(slotSetBackMode(QAction*)));

    QAction * aNone = new QAction(tr("None"), menu);
    aNone->setToolTip(tr("Transparency can be saved to PNG images only."));
    aNone->setProperty("id", 1);
    aNone->setCheckable(true);
    aNone->setActionGroup(m_gBackActions);
    menu->addAction(aNone);

    QAction * aGradient = new QAction(tr("Gradient"), menu);
    aGradient->setProperty("id", 2);
    aGradient->setCheckable(true);
    aGradient->setActionGroup(m_gBackActions);
    menu->addAction(aGradient);

    QAction * aContent = new QAction(tr("Content"), menu);
    aContent->setToolTip(tr("Double click on any content to put it on background."));
    aContent->setEnabled(false);
    aContent->setProperty("id", 3);
    aContent->setCheckable(true);
    aContent->setActionGroup(m_gBackActions);
    menu->addAction(aContent);

    menu->addSeparator();

    QMenu * mScaling = new QMenu(tr("Content Aspect Ratio"), menu);
    m_gBackRatioActions = new QActionGroup(menu);
    connect(m_gBackRatioActions, SIGNAL(triggered(QAction*)), this, SLOT(slotSetBackRatio(QAction*)));
    menu->addMenu(mScaling);

    QAction * aRatioKeepEx = new QAction(tr("Keep proportions by expanding"), mScaling);
    aRatioKeepEx->setProperty("mode", (int)Qt::KeepAspectRatioByExpanding);
    aRatioKeepEx->setCheckable(true);
    aRatioKeepEx->setActionGroup(m_gBackRatioActions);
    mScaling->addAction(aRatioKeepEx);

    QAction * aRatioKeep = new QAction(tr("Keep proportions"), mScaling);
    aRatioKeep->setProperty("mode", (int)Qt::KeepAspectRatio);
    aRatioKeep->setCheckable(true);
    aRatioKeep->setActionGroup(m_gBackRatioActions);
    mScaling->addAction(aRatioKeep);

    QAction * aRatioIgnore = new QAction(tr("Ignore proportions"), mScaling);
    aRatioIgnore->setProperty("mode", (int)Qt::IgnoreAspectRatio);
    aRatioIgnore->setCheckable(true);
    aRatioIgnore->setActionGroup(m_gBackRatioActions);
    mScaling->addAction(aRatioIgnore);

    // initially check the action
    slotBackModeChanged();
    slotBackRatioChanged();
    return menu;
}

QMenu * MainWindow::createDecorationMenu()
{
    QMenu * menu = new QMenu();

    QAction * aTop = new QAction(tr("Top bar"), menu);
    aTop->setCheckable(true);
    aTop->setChecked(m_desk->topBarEnabled());
    connect(aTop, SIGNAL(toggled(bool)), this, SLOT(slotDecoTopBar(bool)));
    menu->addAction(aTop);

    QAction * aBottom = new QAction(tr("Bottom bar"), menu);
    aBottom->setCheckable(true);
    aBottom->setChecked(m_desk->bottomBarEnabled());
    connect(aBottom, SIGNAL(toggled(bool)), this, SLOT(slotDecoBottomBar(bool)));
    menu->addAction(aBottom);

    menu->addSeparator();

    QAction * aSetTitle = new QAction(tr("Set title..."), menu);
    connect(aSetTitle, SIGNAL(triggered()), this, SLOT(slotDecoSetTitle()));
    menu->addAction(aSetTitle);

    QAction * aClearTitle = new QAction(tr("Clear title"), menu);
    connect(aClearTitle, SIGNAL(triggered()), this, SLOT(slotDecoClearTitle()));
    menu->addAction(aClearTitle);

    return menu;
}

QMenu * MainWindow::createOnlineHelpMenu()
{
    QMenu * menu = new QMenu();
    menu->setSeparatorsCollapsible(false);

    m_aHelpTutorial = new QAction(tr("Tutorial Video (0.2)"), menu);
    connect(m_aHelpTutorial, SIGNAL(triggered()), this, SLOT(slotHelpTutorial()));
    menu->addAction(m_aHelpTutorial);

    QAction * aCheckUpdates = new QAction(tr("Check for Updates"), menu);
    connect(aCheckUpdates, SIGNAL(triggered()), this, SLOT(slotHelpUpdates()));
    menu->addAction(aCheckUpdates);

    QAction * aFotowallBlog = new QAction(tr("Fotowall's Blog"), menu);
    connect(aFotowallBlog, SIGNAL(triggered()), this, SLOT(slotHelpBlog()));
    menu->addAction(aFotowallBlog);

    m_aHelpSupport = new QAction("", menu);
    connect(m_aHelpSupport, SIGNAL(triggered()), this, SLOT(slotHelpSupport()));
    menu->addAction(m_aHelpSupport);

    return menu;
}

void MainWindow::createMiscActions()
{
    // select all
    QAction * aSA = new QAction(tr("Select all"), this);
    aSA->setShortcut(tr("CTRL+A"));
    connect(aSA, SIGNAL(triggered()), this, SLOT(slotActionSelectAll()));
    addAction(aSA);
}

void MainWindow::checkForTutorial()
{
    // hide the tutorial link
    m_aHelpTutorial->setVisible(false);

    // try to get the tutorial page (note, multiple QNAMs will be deleted on app closure)
    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotVerifyTutorial(QNetworkReply*)));
    manager->get(QNetworkRequest(TUTORIAL_URL));
}

void MainWindow::checkForSupport()
{
    // hide the support link
    m_aHelpSupport->setVisible(false);

    // check the Open Collaboration Services knowledgebase for Fotowall
    QTimer::singleShot(2000, this, SLOT(slotVerifySupport()));
}

void MainWindow::checkForUpdates()
{
    // find out the time of the last update check
    QDate lastCheck = QSettings().value("fotowall/LastUpdateCheck").toDate();
    if (lastCheck.isNull()) {
        QSettings().setValue("fotowall/LastUpdateCheck", QDate::currentDate());
        return;
    }

    // check for updates 30 days after the last one
    if (lastCheck.daysTo(QDate::currentDate()) > 30)
        QTimer::singleShot(2000, this, SLOT(slotHelpUpdates()));
}

void MainWindow::setNormalProject()
{
    m_modeInfo.setRealSizeInches(-1,-1); // Unset the size (for the saving function)
    static bool skipFirstMaximizeHack = true;
    ui->canvas->setMinimumSize(ui->canvas->minimumSizeHint());
    ui->canvas->setMaximumSize(QSize(16777215, 16777215));
    if (skipFirstMaximizeHack)
        skipFirstMaximizeHack = false;
    else
        showMaximized();
    ui->exportButton->setText(tr("Export"));
    m_desk->setProjectMode(Desk::ModeNormal);
    ui->projectType->setCurrentIndex(0);
}

void MainWindow::setCDProject()
{
    // A CD cover is a 4.75x4.715 inches square.
    m_modeInfo.setRealSizeInches(4.75, 4.75);
    m_modeInfo.setLandscape(false);
    ui->canvas->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print"));
    m_desk->setProjectMode(Desk::ModeCD);
    ui->projectType->setCurrentIndex(1);
}

void MainWindow::setDVDProject()
{
    m_modeInfo.setRealSizeInches((float)10.83, (float)7.2);
    m_modeInfo.setLandscape(true);
    ui->canvas->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print"));
    m_desk->setProjectMode(Desk::ModeDVD);
    ui->projectType->setCurrentIndex(2);
}

void MainWindow::setExactSizeProject()
{
    // Exact size mode
    if(m_modeInfo.realSize().isEmpty()) {
        ExactSizeDialog sizeDialog;
        QPointF screenDpi = m_modeInfo.deskDpi();
        if (screenDpi.x() == screenDpi.y())
            sizeDialog.ui.screenDpi->setValue(screenDpi.x());
        else
            sizeDialog.ui.screenDpi->setSpecialValueText(QString("%1, %2").arg(screenDpi.x()).arg(screenDpi.y()));
        if(sizeDialog.exec() != QDialog::Accepted) {
            return;
        }
        float w = sizeDialog.ui.widthSpinBox->value();
        float h = sizeDialog.ui.heightSpinBox->value();
        int printDpi = sizeDialog.ui.printDpi->value();
        bool landscape = sizeDialog.ui.landscapeCheckBox->isChecked();
        m_modeInfo.setLandscape(landscape);
        m_modeInfo.setPrintDpi(printDpi);
        if(sizeDialog.ui.unityComboBox->currentIndex() == 0)
            m_modeInfo.setRealSizeCm(w, h);
        else
            m_modeInfo.setRealSizeInches(w, h);
    }
    ui->canvas->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print"));
    m_desk->setProjectMode(Desk::ModeExactSize);
    ui->projectType->setCurrentIndex(3);
}

void MainWindow::on_projectType_activated(int index)
{
    m_modeInfo.setRealSizeInches(-1,-1); // Unset the size (so if it is a mode that require
                                        // asking size, it will be asked !
    switch (index) {
        case 0: //Normal project
            setNormalProject();
            break;

        case 1: // CD cover
            setCDProject();
            break;

        case 2: //DVD cover
            setDVDProject();
            break;

        case 3: //Exact Size
            setExactSizeProject();
            break;
    }
}

void MainWindow::on_aAddFlickr_toggled(bool on)
{
    m_desk->setWebContentSelectorVisible(on);
}

void MainWindow::on_aAddPicture_triggered()
{
    // build the extensions list
    QString extensions;
    foreach (const QByteArray & format, QImageReader::supportedImageFormats()) {
        extensions += "*." + format + " *." + format.toUpper() + " ";
    }

    // show the files dialog
    QSettings s;
    QStringList fileNames = QFileDialog::getOpenFileNames(ui->canvas, tr("Select one or more pictures to add"), s.value("fotowall/loadImagesDir").toString(), tr("Images (%1)").arg(extensions));
    if (!fileNames.isEmpty()) {
        m_desk->addPictures(fileNames);
        QFileInfo path(fileNames[0]);
        s.setValue("fotowall/loadImagesDir", path.absolutePath());
    }
}

void MainWindow::on_aAddText_triggered()
{
    m_desk->addTextContent();
}

void MainWindow::on_aAddWebcam_triggered()
{
    m_desk->addWebcamContent(0);
}

#ifdef QT_OPENGL_LIB
#include <QGLWidget>
void MainWindow::on_accelBox_toggled(bool opengl)
{
    QStyle * style = ui->canvas->viewport()->style();
    // set OpenGL viewport
    if (opengl) {
        WarningBox("SkipWarnings/opengl", tr("OpenGL"), tr("OpenGL accelerates graphics. However it's not guaranteed that it will work on your system. Just try and see if it works for you ;-)<br> - if it feels slower, make sure that your driver accelerates OpenGL<br> - if Fotowall stops responding after switching to OpenGL, just don't use this feature next time<br><br>NOTE: OpenGL doesn't work with 'Transparent' mode.<br>"));
        ui->transpBox->setChecked(false);
        ui->canvas->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        ui->canvas->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    }
    // set Normal viewport
    else {
        ui->canvas->setViewport(new QWidget());
        ui->canvas->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    }
    ui->canvas->viewport()->setStyle(style);
    update();
}
#else
void MainWindow::on_accelBox_toggled(bool) {}
#endif

void MainWindow::on_transpBox_toggled(bool transparent)
{
#if QT_VERSION >= 0x040500
    if (!m_windowFlags)
        m_windowFlags = windowFlags();
    if (transparent) {
        // one-time warning
        WarningBox("SkipWarnings/transparency", tr("Transparency"), tr("This feature has not been widely tested yet.<br> - on linux it requires compositing (like compiz/beryl, kwin4)<br> - on windows and mac it seems to work<br>If you see a black background then transparency is not supported on your system.<br><br>NOTE: you should set the 'Transparent' Background to notice the the window transparency.<br>"));

        // go transparent
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);

        // hint the render that we're transparent now
        RenderOpts::ARGBWindow = true;

#ifdef Q_OS_WIN
        // needed on windows for translucency
        setWindowFlags(m_windowFlags | Qt::FramelessWindowHint);
        show();
#endif

        // set 'NoBackground' to show that we're transparent for real
        m_desk->setBackMode(1);
    } else {
        // back to normal (non-alphaed) window
        setAttribute(Qt::WA_TranslucentBackground, false);
        setAttribute(Qt::WA_NoSystemBackground, false);

#ifdef Q_OS_WIN
        // disable no-border on windows
        setWindowFlags(m_windowFlags);
        show();
#endif

        // hint the render that we're opaque again
        RenderOpts::ARGBWindow = false;
    }
    // refresh the window
    update();
#else
    Q_UNUSED(transparent)
#endif
}

void MainWindow::on_introButton_clicked()
{
    m_desk->showIntroduction();
}

void MainWindow::on_loadButton_clicked()
{
    QSettings s;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select the Fotowall file"), s.value("fotowall/loadProjectDir").toString(), tr("Fotowall (*.fotowall)"));
    if (fileName.isNull())
        return;

    QFileInfo path(fileName);
    s.setValue("fotowall/loadProjectDir", path.absolutePath());
    loadXml(fileName);
}

void MainWindow::on_saveButton_clicked()
{
    QSettings s;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select the Fotowall file"), s.value("fotowall/saveProjectDir").toString(), "Fotowall (*.fotowall)");
    if (fileName.isNull())
        return;
    QFileInfo path(fileName);
    s.setValue("fotowall/saveProjectDir", path.absolutePath());

    if (!fileName.endsWith(".fotowall", Qt::CaseInsensitive))
        fileName += ".fotowall";
    saveXml(fileName);
}

void MainWindow::on_exportButton_clicked()
{
    // show the Export Wizard on normal mode
    if (m_desk->projectMode() == Desk::ModeNormal) {
        ExportWizard(m_desk).exec();
        return;
    }

    // print on other modes
    m_desk->printAsImage(m_modeInfo.printDpi(), m_modeInfo.printPixelSize(), m_modeInfo.landscape());
}
/*
void MainWindow::on_quitButton_clicked()
{
    QCoreApplication::quit();
}
*/
void MainWindow::slotActionSelectAll()
{
    m_desk->selectAllContent();
}

void MainWindow::slotArrangeForceField(bool checked)
{
    m_desk->setForceFieldEnabled(checked);
}

void MainWindow::slotDecoTopBar(bool checked)
{
    m_desk->setTopBarEnabled(checked);
}

void MainWindow::slotDecoBottomBar(bool checked)
{
    m_desk->setBottomBarEnabled(checked);
}

void MainWindow::slotDecoSetTitle()
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

void MainWindow::slotDecoClearTitle()
{
    m_desk->setTitleText(QString());
}

void MainWindow::slotHelpBlog()
{
    int answer = QMessageBox::question(this, tr("Opening Fotowall's author Blog"), tr("This is the blog of the main author of Fotowall.\nYou can find some news while we set up a proper website ;-)\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QDesktopServices::openUrl(ENRICOBLOG_URL);
}

void MainWindow::slotHelpSupport()
{
}

void MainWindow::slotHelpTutorial()
{
    int answer = QMessageBox::question(this, tr("Opening the Web Tutorial"), tr("The Tutorial is provided on Fosswire by Peter Upfold.\nIt's about Fotowall 0.2 a rather old version.\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QDesktopServices::openUrl(TUTORIAL_URL);
}

void MainWindow::slotHelpUpdates()
{
    VersionCheckDialog vcd;
    vcd.exec();
    QSettings().setValue("fotowall/LastUpdateCheck", QDate::currentDate());
}

void MainWindow::slotSetBackMode(QAction* action)
{
    int choice = action->property("id").toUInt();
    m_desk->setBackMode(choice);
}

void MainWindow::slotSetBackRatio(QAction* action)
{
    Qt::AspectRatioMode mode = (Qt::AspectRatioMode)action->property("mode").toInt();
    m_desk->setBackContentRatio(mode);
}

void MainWindow::slotBackModeChanged()
{
    int mode = m_desk->backMode();
    m_gBackActions->actions()[mode - 1]->setChecked(true);
    m_gBackActions->actions()[2]->setEnabled(mode == 3);
}

void MainWindow::slotBackRatioChanged()
{
    Qt::AspectRatioMode mode = m_desk->backContentRatio();
    if (mode == Qt::KeepAspectRatioByExpanding)
        m_gBackRatioActions->actions()[0]->setChecked(true);
    else if (mode == Qt::KeepAspectRatio)
        m_gBackRatioActions->actions()[1]->setChecked(true);
    else if (mode == Qt::IgnoreAspectRatio)
        m_gBackRatioActions->actions()[2]->setChecked(true);
}

void MainWindow::slotShowPropertiesWidget(QWidget * widget)
{
    // delete current Properties content
    QLayoutItem * prevItem = ui->propLayout->takeAt(0);
    if (prevItem) {
        delete prevItem->widget();
        delete prevItem;
    }

    // show the Properties container with new content and title
    if (widget) {
        ui->widgetCanvas->collapse();
        widget->setParent(ui->widgetProperties);
        ui->propLayout->addWidget(widget);
        ui->widgetProperties->setTitle(widget->windowTitle());
        ui->widgetProperties->expand();
    }
    // or show the Canvas containter
    else {
        ui->widgetProperties->collapse();
        ui->widgetCanvas->expand();
    }
}

void MainWindow::slotVerifyTutorial(QNetworkReply * reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;

    QString htmlCode = reply->readAll();
    bool tutorialValid = htmlCode.contains(TUTORIAL_STRING, Qt::CaseInsensitive);
    m_aHelpTutorial->setVisible(tutorialValid);
}

void MainWindow::slotVerifySupport(/*const KnowledgeItemV1List & items*/)
{
    int supportEntries = 0;
    m_aHelpSupport->setVisible(supportEntries > 0);
    m_aHelpSupport->setText(tr("Support (%1)").arg(supportEntries));
/*
    qWarning("MainWindow::slotOcsKbItems: got %d items", items.size());
    foreach (KnowledgeItemV1 * item, items) {
        qWarning() << item->name() << item->description() << item->answer();
    }
*/
}

void MainWindow::slotVerifyVideoInputs(int count)
{
    // maybe blink or something?
    ui->aAddWebcam->setVisible(count > 0);
}
