/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "App/MainWindow.h"

#include "3rdparty/likebackfrontend/LikeBack.h"
#include "Canvas/CanvasModeInfo.h"
#include "Canvas/Canvas.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/MetaXmlReader.h"
#include "Shared/RenderOpts.h"
#include "App.h"
#include "CanvasAppliance.h"
#include "FotowallFile.h"
#include "Hardware3DTest.h"
#include "SceneView.h"
#include "Settings.h"
#include "VersionCheckDialog.h"
#include "WordcloudAppliance.h"
#include "ui_MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

// current location and 'check string' for the tutorial
#define TUTORIAL_URL QUrl("http://fosswire.com/post/2008/09/fotowall-make-wallpaper-collages-from-your-photos/")
#define TUTORIAL_STRING "Peter walks you through how to use Foto"
#define ENRICOBLOG_STRING "http://www.enricoros.com/blog/tag/fotowall/"
#define FOTOWALL_FEEDBACK_LANGS "en,it,fr"
#define FOTOWALL_FEEDBACK_SERVER "www.enricoros.com"
#define FOTOWALL_FEEDBACK_PATH "/opensource/fotowall/feedback/send.php"


MainWindow::MainWindow(const QStringList & contentUrls, QWidget * parent)
    : Appliance::Container(parent)
    , ui(new Ui::MainWindow())
    , m_appManager(new Appliance::Manager)
    , m_likeBack(0)
    , m_aHelpTutorial(0)
    , m_applyingAccelState(false)
{
    // setup widget
#if QT_VERSION >= 0x040500
    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion());
#else
    setWindowTitle(QCoreApplication::applicationName() + " " + QCoreApplication::applicationVersion() + "   -Limited Edition (Qt 4.4)-");
#endif
    setWindowIcon(QIcon(":/data/fotowall.png"));

    // init ui
    ui->setupUi(this);
    ui->sceneView->setFocus();
    ui->onlineHelpButton->setMenu(createOnlineHelpMenu());
    ui->sceneView->addOverlayWidget(ui->applianceNavBar);
#if QT_VERSION >= 0x040500
    ui->transpBox->setEnabled(true);
    ui->accelBox->setEnabled(ui->sceneView->supportsOpenGL());
    ui->accelTestButton->setEnabled(ui->sceneView->supportsOpenGL());
#endif
    connect(ui->sceneView, SIGNAL(heavyRepaint()), this, SLOT(slotRenderingSlow()));
    createLikeBack();

    // init the Appliance Manager
    m_appManager->setContainer(this);
    connect(m_appManager, SIGNAL(structureChanged()), this, SLOT(slotApplianceStructureChanged()));
    connect(ui->applianceNavBar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotApplianceClicked(quint32)));
    ui->applianceSidebar->hide();

    // show (with last geometry)
    if (!restoreGeometry(App::settings->value("Fotowall/Geometry").toByteArray())) {
        QRect desktopGeometry = QApplication::desktop()->availableGeometry();
        resize(2 * desktopGeometry.width() / 3, 2 * desktopGeometry.height() / 3);
        showMaximized();
    } else
        show();

    // create a Canvas (and load/populate it)
    Canvas * canvas = new Canvas(ui->sceneView->size(), this);
        // open if single fotowall file
        if (contentUrls.size() == 1 && App::isFotowallFile(contentUrls.first()))
            FotowallFile::read(contentUrls.first(), canvas);

        // add if many pictures
        else if (!contentUrls.isEmpty())
            canvas->addPictureContent(contentUrls);

        // no url: display history
#if 0
        else {
            foreach (const QUrl & url, App::settings->recentFotowallUrls())
                canvas->addCanvasViewContent(QStringList() << url.toString());
        }
#endif
    // use the editing appliance over it
    editCanvas(canvas);

    // check stuff on the net
    checkForTutorial();
    checkForUpdates();

    // the first time, show the introduction
    if (App::settings->firstTime())
        on_introButton_clicked();
}

MainWindow::~MainWindow()
{
    // save window geometry
    if (!isMaximized() && !isFullScreen())
        App::settings->setValue("Fotowall/Geometry", saveGeometry());
    else
        App::settings->remove("Fotowall/Geometry");

    // this is an example of 'autosave-like function'
    //QString tempPath = QDir::tempPath() + QDir::separator() + "autosave.fotowall";
    //FotowallFile::saveV2(tempPath, m_canvas);

    // delete everything
    // m_aHelpTutorial is deleted by its menu (that's parented to this)
    delete m_appManager;
    delete m_likeBack;
    delete ui;
}

void MainWindow::editCanvas(Canvas * canvas)
{
    CanvasAppliance * cApp = new CanvasAppliance(canvas, ui->sceneView->physicalDpiX(), ui->sceneView->physicalDpiY(), this);
    m_appManager->stackAppliance(cApp);
}

void MainWindow::editWordcloud(Wordcloud::Cloud * cloud)
{
    WordcloudAppliance * wApp = new WordcloudAppliance(cloud, this);
    m_appManager->stackAppliance(wApp);
}

void MainWindow::applianceSetScene(AbstractScene * scene)
{
    ui->sceneView->setScene(scene);
}

static void hideInLayout(QLayout * layout)
{
    while (QLayoutItem * item = layout->takeAt(0)) {
        if (QWidget * oldWidget = item->widget())
            oldWidget->setVisible(false);
        delete item;
    }
}

void MainWindow::applianceSetTopbar(const QList<QWidget *> & widgets)
{
    // clear the topbar layout hiding all widgets
    hideInLayout(ui->applianceTopbarLayout);

    // add the widgets to the topbar and show them
    foreach (QWidget * widget, widgets) {
        ui->applianceTopbarLayout->addWidget(widget);
        widget->setVisible(true);
    }
}

void MainWindow::applianceSetSidebar(QWidget * widget)
{
    // clear the sidebar layout hiding any widget
    hideInLayout(ui->applianceSidebarLayout);

    // completely hide the sidebar if no widget
    ui->applianceSidebar->setVisible(widget);

    // if any, add the widget to the sidebar
    if (widget) {
        ui->applianceSidebarLayout->addWidget(widget);
        widget->show();
    }
}

void MainWindow::applianceSetCentralwidget(QWidget * widget)
{
    if (widget)
        qWarning("MainWindow::applianceSetCentralwidget: unsupported");
}

void MainWindow::applianceSetValue(quint32 id, const QVariant & value)
{
    switch (id) {
        case App::CV_ExportPrint: {
            bool valueSent = !value.isNull();
            ui->exportButton->setVisible(valueSent);
            ui->introButton->setEnabled(valueSent);
            ui->saveButton->setEnabled(valueSent);
            if (!value.isNull()) {
                ui->exportButton->setText(value.toBool() ? tr("Print") : tr("Export"));
                ui->exportButton->setProperty("printing", value.toBool());
            }
            }break;

         default:
            qWarning("MainWindow::applianceSetValue: unknown id 0x%x", id);
            break;
    }
}

// ###
void MainWindow::closeEvent(QCloseEvent * event)
{
    // build the closure dialog
    ButtonsDialog quitAsk("MainWindow-Exit", tr("Closing Fotowall..."));
    quitAsk.setMinimumWidth(350);
    quitAsk.setButtonText(QDialogButtonBox::Cancel, tr("Cancel"));
#if 0
    if (m_canvas && m_canvas->pendingChanges()) {
        quitAsk.setMessage(tr("Are you sure you want to quit and lose your changes?"));
        quitAsk.setButtonText(QDialogButtonBox::Save, tr("Save"));
        quitAsk.setButtonText(QDialogButtonBox::Close, tr("Don't Save"));
        quitAsk.setButtons(QDialogButtonBox::Save | QDialogButtonBox::Close | QDialogButtonBox::Cancel);
    } else {
#endif
        quitAsk.setMessage(tr("Are you sure you want to quit?"));
        quitAsk.setButtonText(QDialogButtonBox::Close, tr("Quit"));
        quitAsk.setButtons(QDialogButtonBox::Close | QDialogButtonBox::Cancel);
#if 0
    }
#endif

    // react to the dialog's answer
    switch (quitAsk.execute()) {
        case QDialogButtonBox::Cancel:
            event->ignore();
            break;
#if 0
        case QDialogButtonBox::Save:
            // save file and return to Fotowall if canceled
            if (!on_saveButton_clicked()) {
                event->ignore();
                break;
            }
            // fall through
#endif
        default:
            event->accept();
            break;
    }
}

QMenu * MainWindow::createOnlineHelpMenu()
{
    QMenu * menu = new QMenu(this);
    menu->setSeparatorsCollapsible(false);

    m_aHelpTutorial = new QAction(tr("Tutorial Video (0.2)"), menu);
    connect(m_aHelpTutorial, SIGNAL(triggered()), this, SLOT(slotHelpTutorial()));
    menu->addAction(m_aHelpTutorial);

    QAction * aCheckUpdates = new QAction(tr("Check for Updates"), menu);
    connect(aCheckUpdates, SIGNAL(triggered()), this, SLOT(slotHelpUpdates()));
    menu->addAction(aCheckUpdates);

    QAction * aFotowallBlog = new QAction(tr("Fotowall's Blog"), menu);
    connect(aFotowallBlog, SIGNAL(triggered()), this, SLOT(slotHelpWebsite()));
    menu->addAction(aFotowallBlog);

    return menu;
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

void MainWindow::checkForUpdates()
{
    // find out the time of the last update check
    QDate lastCheck = App::settings->value("Fotowall/LastUpdateCheck").toDate();
    if (lastCheck.isNull()) {
        App::settings->setValue("Fotowall/LastUpdateCheck", QDate::currentDate());
        return;
    }

    // check for updates 30 days after the last one
    if (lastCheck.daysTo(QDate::currentDate()) > 30)
        QTimer::singleShot(2000, this, SLOT(slotHelpUpdates()));
}

void MainWindow::createLikeBack()
{
    m_likeBack = new LikeBack(LikeBack::AllButtons, false, this);
    m_likeBack->setAcceptedLanguages(QString(FOTOWALL_FEEDBACK_LANGS).split(","));
    m_likeBack->setServer(FOTOWALL_FEEDBACK_SERVER, FOTOWALL_FEEDBACK_PATH);
}

void MainWindow::slotApplianceClicked(quint32 id)
{
    m_appManager->dropStackAfter(id - 1);
}

void MainWindow::slotApplianceStructureChanged()
{
    // build the new breadcrumbbar's contents
    ui->applianceNavBar->clearNodes();
    QList<Appliance::AbstractAppliance *> appliances = m_appManager->stackedAppliances();
    if (appliances.size() >= 2) {
        quint32 index = 0;
        foreach (Appliance::AbstractAppliance * app, appliances) {
            ui->applianceNavBar->addNode(index + 1, app->applianceName(), index);
            index++;
        }
    }

    // repaint all
    update();
}

void MainWindow::slotRenderingSlow()
{
    // don't act anymore
    disconnect(ui->sceneView, SIGNAL(heavyRepaint()), this, SLOT(slotRenderingSlow()));

    // draw attenction to the testing button
    if (ui->sceneView->supportsOpenGL()) {
        ui->modeWidget->setChecked(true);
        ui->accelTestButton->drawAttenction();
    }
}

bool MainWindow::on_loadButton_clicked()
{
    // make up the default load path (stored as 'Fotowall/LoadProjectDir')
    QString defaultLoadPath = App::settings->value("Fotowall/LoadProjectDir").toString();

    // ask the file name, validate it, store back to settings and load the file
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select the Fotowall file"), defaultLoadPath, tr("Fotowall (*.fotowall)"));
    if (fileName.isNull())
        return false;
    App::settings->setValue("Fotowall/LoadProjectDir", QFileInfo(fileName).absolutePath());

    // try to load the canvas
    Canvas * canvas = new Canvas(ui->sceneView->size(), this);
    if (!FotowallFile::read(fileName, canvas)) {
        delete canvas;
        return false;
    }

    // close all and edit the loaded file
    m_appManager->clearAppliances();
    editCanvas(canvas);
    return true;
}

bool MainWindow::on_saveButton_clicked()
{
    return m_appManager->currentApplianceCommand(App::AC_Save);
}

void MainWindow::on_exportButton_clicked()
{
    m_appManager->currentApplianceCommand(App::AC_Export);
}

void MainWindow::on_introButton_clicked()
{
    m_appManager->currentApplianceCommand(App::AC_ShowIntro);
}

void MainWindow::on_lbLike_clicked()
{
    m_likeBack->execCommentDialog(LikeBack::Like);
}

void MainWindow::on_lbDislike_clicked()
{
    m_likeBack->execCommentDialog(LikeBack::Dislike);
}

void MainWindow::on_lbFeature_clicked()
{
    m_likeBack->execCommentDialog(LikeBack::Feature);
}

void MainWindow::on_lbBug_clicked()
{
    m_likeBack->execCommentDialog(LikeBack::Bug);
}

void MainWindow::slotHelpWebsite()
{
    // start a fetch if no URL has been determined
    if (m_website.isEmpty()) {
        MetaXml::Connector * conn = new MetaXml::Connector();
        connect(conn, SIGNAL(fetched()), this, SLOT(slotHelpWebsiteFetched()));
        connect(conn, SIGNAL(fetchError(const QString &)), this, SLOT(slotHelpWebsiteFetchError()));
        return;
    }

    // open the website
    if (QMessageBox::question(this, tr("Opening Fotowall's author Blog"), tr("This is the blog of the main author of Fotowall.\nYou can find some news while we set up a proper website ;-)\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        QDesktopServices::openUrl(QUrl(m_website));
}

void MainWindow::slotHelpWebsiteFetched()
{
    // get the websites from the conn
    MetaXml::Connector * conn = dynamic_cast<MetaXml::Connector *>(sender());
    if (conn && !conn->reader()->websites.isEmpty()) {
        m_website = conn->reader()->websites.first().url;
        if (!m_website.isEmpty()) {
            slotHelpWebsite();
            return;
        }
    }

    // catch-all condition: use default url
    slotHelpWebsiteFetchError();
}

void MainWindow::slotHelpWebsiteFetchError()
{
    m_website = ENRICOBLOG_STRING;
    slotHelpWebsite();
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
    App::settings->setValue("Fotowall/LastUpdateCheck", QDate::currentDate());
}

void MainWindow::slotVerifyTutorial(QNetworkReply * reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;

    QString htmlCode = reply->readAll();
    bool tutorialValid = htmlCode.contains(TUTORIAL_STRING, Qt::CaseInsensitive);
    m_aHelpTutorial->setVisible(tutorialValid);
}

bool MainWindow::on_accelTestButton_clicked()
{
    // ask for confirmation
    if (!m_applyingAccelState) {
        ButtonsDialog d("TestOpenGL", tr("Accelerated Rendering"), tr("OpenGL accelerates graphics. However it's not supported by every system.<br><b>Do you want to do an acceleration test?</b>"), QDialogButtonBox::Ok | QDialogButtonBox::Cancel, true, false);
        d.setIcon(QStyle::SP_MessageBoxQuestion);
        if (d.execute() == QDialogButtonBox::Cancel)
            return false;
    }

    // run the Hardware Test
    Hardware3DTest testDialog;
    Hardware3DTest::ExitState choice = testDialog.run();
    if (choice == Hardware3DTest::Canceled)
        return false;

    // apply the user choice
    bool setGL = choice == Hardware3DTest::UseOpenGL;
    App::settings->setValue("OpenGL/Tested", true);
    App::settings->setValue("OpenGL/TestResult", setGL);
    m_applyingAccelState = true;
    ui->accelBox->setChecked(setGL);
    m_applyingAccelState = false;
    return setGL;
}

void MainWindow::on_accelBox_toggled(bool checked)
{
    // ask for confirmation/testing when enabling opengl
    if (checked && !m_applyingAccelState) {
        bool tested = App::settings->contains("OpenGL/Tested");

        QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel | (tested ? QDialogButtonBox::NoButton : QDialogButtonBox::Retry);
        ButtonsDialog input("EnableOpenGL", tr("OpenGL"), tr("OpenGL accelerates graphics, but it doesn't work on some systems.<br> - if it feels slower, make sure that your driver accelerates OpenGL<br> - if Fotowall stops responding after switching to OpenGL, don't enable OpenGL next time"), buttons, true, tested);
        input.setIcon(QStyle::SP_MessageBoxInformation);
        if (!tested)
            input.setButtonText(QDialogButtonBox::Retry, tr("Test OpenGL"));
        QDialogButtonBox::StandardButton choice = input.execute();

        // action canceled
        if (choice == QDialogButtonBox::Cancel) {
            ui->accelBox->setChecked(false);
            return;
        }

        // testing required
        if (choice == QDialogButtonBox::Retry) {
            m_applyingAccelState = true;
            checked = on_accelTestButton_clicked();
            m_applyingAccelState = false;
        }
    }

#if QT_VERSION < 0x040600
    // WORKAROUND Qt <= 4.6-beta1: toggle transparency with opengl
    ui->transpBox->setChecked(false);
#endif

    // set opengl state
    ui->sceneView->setOpenGL(checked);

    // save opengl state
    RenderOpts::OpenGLWindow = ui->sceneView->openGL();
}

#ifdef Q_WS_WIN
/**
  Blur behind windows (on Windows Vista/7)

  The following code snippet has been borrowed from Jens of Qt Software / Nokia
  see: http://labs.qt.nokia.com/blogs/2009/09/15/using-blur-behind-on-windows/
  the license says: Use, modification and distribution is allowed without
  limitation, warranty, liability or support of any kind.
**/
#include <QLibrary>
#include <qt_windows.h>

// Dwm Data Structures
#define DWM_BB_ENABLE                 0x00000001  // fEnable has been specified
typedef struct _DWM_BLURBEHIND
{
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;

// Dwm entry points
typedef HRESULT (WINAPI *PtrDwmIsCompositionEnabled)(BOOL * pfEnabled);
typedef HRESULT (WINAPI *PtrDwmEnableBlurBehindWindow)(HWND hWnd, const DWM_BLURBEHIND * pBlurBehind);
static PtrDwmIsCompositionEnabled pDwmIsCompositionEnabled = 0;
static PtrDwmEnableBlurBehindWindow pDwmEnableBlurBehindWindow  = 0;

static bool dwmResolveLibs()
{
    if (!pDwmIsCompositionEnabled) {
        QLibrary dwmLib(QString::fromAscii("dwmapi"));
        pDwmIsCompositionEnabled = (PtrDwmIsCompositionEnabled)dwmLib.resolve("DwmIsCompositionEnabled");
        pDwmEnableBlurBehindWindow = (PtrDwmEnableBlurBehindWindow)dwmLib.resolve("DwmEnableBlurBehindWindow");
    }
    return pDwmIsCompositionEnabled != 0;
}

static bool dwmEnableBlurBehindWindow(QWidget * widget, bool enable)
{
    bool result = false;
    if (dwmResolveLibs()) {
        DWM_BLURBEHIND bb = {0};
        bb.dwFlags = DWM_BB_ENABLE;
        bb.fEnable = enable;
        bb.hRgnBlur = NULL;
        HRESULT hr = pDwmEnableBlurBehindWindow(widget->winId(), &bb);
        if (SUCCEEDED(hr))
            result = true;
    }
    return result;
}
#endif

void MainWindow::on_transpBox_toggled(bool transparent)
{
#if QT_VERSION >= 0x040500
#ifdef Q_OS_WIN
    static Qt::WindowFlags initialWindowFlags = windowFlags();
#endif
    if (transparent) {
#ifdef Q_OS_LINUX
        // one-time warning
        ButtonsDialog warning("EnableTransparency", tr("Transparency"), tr("This feature requires compositing (compiz or kwin4) to work on Linux.<br>If you see a black background then transparency is not supported on your system."), QDialogButtonBox::Ok, true, true);
        warning.setIcon(QStyle::SP_MessageBoxInformation);
        warning.execute();
#endif

#if QT_VERSION < 0x040600
        // WORKAROUND Qt <= 4.6-beta1: toggle opengl with transparency
        ui->accelBox->setChecked(false);
#endif

        // go transparent
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);

        // hint the render that we're transparent now
        RenderOpts::ARGBWindow = true;

#ifdef Q_OS_WIN
        // enable blur behind on Vista/7
        if (!dwmEnableBlurBehindWindow(this, true)) {
            // if blur fails, use a frameless window that's needed on XP for transparency
            setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            show();
        }
#endif

        // disable appliance background too
        m_appManager->currentApplianceCommand(App::AC_ClearBackground);
    } else {
        // back to normal (non-alphaed) window
        setAttribute(Qt::WA_TranslucentBackground, false);
        setAttribute(Qt::WA_NoSystemBackground, false);

#ifdef Q_OS_WIN
        // disable no-border on windows
        setWindowFlags(initialWindowFlags);
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
