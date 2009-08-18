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
#include "items/VideoProvider.h"
#include "Desk.h"
#include "ExactSizeDialog.h"
#include "ExportWizard.h"
#include "ModeInfo.h"
#include "VersionCheckDialog.h"
#include "XmlRead.h"
#include "XmlSave.h"
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
#include <QPushButton>
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
#include "ui_FotoWall.h"


FotoWall::FotoWall(QWidget * parent)
    : QWidget(parent)
    , ui(new Ui::FotoWall())
    , m_desk(0)
    , m_aHelpTutorial(0)
    , m_aHelpSupport(0)
    , m_gBackActions(0)
{
    // setup widget
    QRect geom = QApplication::desktop()->availableGeometry();
    resize(2 * geom.width() / 3, 2 * geom.height() / 3);
    setWindowTitle(qApp->applicationName() + " " + qApp->applicationVersion());
    setWindowIcon(QIcon(":/data/fotowall.png"));
#if 0 //QT_VERSION >= 0x040500
    // this produces cool results, but it's premature for something this heavy
    setAttribute(Qt::WA_TranslucentBackground, true);
#endif

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
    ui->b3->setDefaultAction(ui->aAddVideo);
    ui->b4->setDefaultAction(ui->aAddFlickr);
#ifdef HAS_OPENGL
    ui->accelBox->setChecked(false);
#else
    ui->accelBox->hide();
#endif
    ui->widgetProperties->collapse();
    ui->widgetCanvas->expand();

    // attach menus
    ///ui->arrangeButton->setMenu(createArrangeMenu());
    ui->backButton->setMenu(createBackgroundMenu());
    ui->decoButton->setMenu(createDecorationMenu());
    ui->howtoButton->setMenu(createHelpMenu());

    // react to VideoProvider
    ui->aAddVideo->setVisible(VideoProvider::instance()->inputCount() > 0);
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
}

FotoWall::~FotoWall()
{
    // dump current layout
    saveXml(QDir::tempPath() + QDir::separator() + "autosave.fotowall");

    // delete everything
    delete m_desk;
    delete ui;
}

void FotoWall::setModeInfo(ModeInfo modeInfo)
{
    m_modeInfo = modeInfo;
    m_modeInfo.setDeskDpi(ui->canvas->logicalDpiX(), ui->canvas->logicalDpiY());
}

ModeInfo FotoWall::getModeInfo()
{
    return m_modeInfo;
}

void FotoWall::restoreMode(int mode)
{
    if (mode == 3) { // If exact size project
        // Called here not to have the unneeded size dialog
        setExactSizeProject();
    } else {
        on_projectType_activated(mode);
    }
}

void FotoWall::loadXml(const QString & filePath)
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

void FotoWall::saveXml(const QString & filePath) const
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

void FotoWall::showIntroduction()
{
    m_desk->showIntroduction();
}

void FotoWall::loadImages(QStringList &imagesPath)
{
    m_desk->addPictures(imagesPath);
}

QMenu * FotoWall::createArrangeMenu()
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

QMenu * FotoWall::createBackgroundMenu()
{
    QMenu * menu = new QMenu();

    m_gBackActions = new QActionGroup(menu);
    connect(m_gBackActions, SIGNAL(triggered(QAction*)), this, SLOT(slotSetBackMode(QAction*)));

    QAction * aGradient = new QAction(tr("Gradient"), menu);
    aGradient->setProperty("id", 1);
    aGradient->setCheckable(true);
    aGradient->setActionGroup(m_gBackActions);
    menu->addAction(aGradient);

    QAction * aTransparent = new QAction(tr("Transparent"), menu);
    aTransparent->setToolTip(tr("Transparency can be saved to PNG images only."));
    aTransparent->setProperty("id", 2);
    aTransparent->setCheckable(true);
    aTransparent->setActionGroup(m_gBackActions);
    menu->addAction(aTransparent);

    QAction * aContent = new QAction(tr("Content"), menu);
    aContent->setToolTip(tr("Double click on any content to put it on background."));
    aContent->setEnabled(false);
    aContent->setProperty("id", 3);
    aContent->setCheckable(true);
    aContent->setActionGroup(m_gBackActions);
    menu->addAction(aContent);

    // initially check the action
    slotBackModeChanged();
    return menu;
}

QMenu * FotoWall::createDecorationMenu()
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
    aSetTitle->setShortcut(tr("CTRL+T"));
    connect(aSetTitle, SIGNAL(triggered()), this, SLOT(slotDecoSetTitle()));
    menu->addAction(aSetTitle);

    QAction * aClearTitle = new QAction(tr("Clear title"), menu);
    connect(aClearTitle, SIGNAL(triggered()), this, SLOT(slotDecoClearTitle()));
    menu->addAction(aClearTitle);

    return menu;
}

QMenu * FotoWall::createHelpMenu()
{
    QMenu * menu = new QMenu();
    menu->setSeparatorsCollapsible(false);

    QAction * aSep = new QAction(tr("About"), menu);
    aSep->setSeparator(true);
    menu->addAction(aSep);

    QAction * aIntroduction = new QAction(tr("Introduction"), menu);
    connect(aIntroduction, SIGNAL(triggered()), this, SLOT(slotHelpIntroduction()));
    menu->addAction(aIntroduction);

    QAction * aSep2 = new QAction(tr("Internet Resources"), menu);
    aSep2->setSeparator(true);
    menu->addAction(aSep2);

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

void FotoWall::createMiscActions()
{
    // select all
    QAction * aSA = new QAction(tr("Select all"), this);
    aSA->setShortcut(tr("CTRL+A"));
    connect(aSA, SIGNAL(triggered()), this, SLOT(slotActionSelectAll()));
    addAction(aSA);
}

void FotoWall::checkForTutorial()
{
    // hide the tutorial link
    m_aHelpTutorial->setVisible(false);

    // try to get the tutorial page (note, multiple QNAMs will be deleted on app closure)
    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotVerifyTutorial(QNetworkReply*)));
    manager->get(QNetworkRequest(TUTORIAL_URL));
}

void FotoWall::checkForSupport()
{
    // hide the support link
    m_aHelpSupport->setVisible(false);

    // check the Open Collaboration Services knowledgebase for FotoWall
    // TODO
    QTimer::singleShot(2000, this, SLOT(slotVerifySupport()));
}

void FotoWall::setNormalProject()
{
    m_modeInfo.setRealSizeInches(-1,-1); // Unset the size (for the saving function)
    static bool skipFirstMaximizeHack = true;
    ui->canvas->setMinimumSize(ui->canvas->minimumSizeHint());
    ui->canvas->setMaximumSize(QSize(16777215, 16777215));
    if (skipFirstMaximizeHack)
        skipFirstMaximizeHack = false;
    else
        showMaximized();
    ui->exportButton->setText(tr("export"));
    m_desk->setProjectMode(Desk::ModeNormal);
    ui->projectType->setCurrentIndex(0);
}

void FotoWall::setCDProject()
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

void FotoWall::setDVDProject()
{
    m_modeInfo.setRealSizeInches((float)10.83, (float)7.2);
    m_modeInfo.setLandscape(true);
    ui->canvas->setFixedSize(m_modeInfo.deskPixelSize());
    showNormal();
    ui->exportButton->setText(tr("print"));
    m_desk->setProjectMode(Desk::ModeDVD);
    ui->projectType->setCurrentIndex(2);
}

void FotoWall::setExactSizeProject()
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

void FotoWall::on_projectType_activated(int index)
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

void FotoWall::on_aAddFlickr_toggled(bool on)
{
    m_desk->setWebContentSelectorVisible(on);
}

void FotoWall::on_aAddPicture_triggered()
{
    // build the extensions list
    QString extensions;
    foreach (const QByteArray & format, QImageReader::supportedImageFormats()) {
        extensions += "*." + format + " *." + format.toUpper() + " ";
    }

    // show the files dialog
    QStringList fileNames = QFileDialog::getOpenFileNames(ui->canvas, tr("Select one or more pictures to add"), QString(), tr("Images (%1)").arg(extensions));
    if (!fileNames.isEmpty())
        m_desk->addPictures(fileNames);
}

void FotoWall::on_aAddText_triggered()
{
    m_desk->addTextContent();
}

void FotoWall::on_aAddVideo_triggered()
{
    m_desk->addVideoContent(0);
}

#ifdef HAS_OPENGL
#include <QGLWidget>
#endif
void FotoWall::on_accelBox_toggled(bool checked)
{
    if (checked) {
        // 3D MODE
#ifdef HAS_OPENGL
        ui->canvas->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
        ui->canvas->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
#endif
    } else {
        // normal mode
        ui->canvas->setViewport(new QWidget());
        ui->canvas->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    }
}

void FotoWall::on_loadButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select FotoWall file"), QString(), tr("FotoWall (*.fotowall)"));
    loadXml(fileName);
}

void FotoWall::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select FotoWall file"), QString(), "FotoWall (*.fotowall)");
    if (fileName.isNull())
        return;
    if (!fileName.endsWith(".fotowall", Qt::CaseInsensitive))
        fileName += ".fotowall";
    saveXml(fileName);
}

void FotoWall::on_exportButton_clicked()
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
void FotoWall::on_quitButton_clicked()
{
    QCoreApplication::quit();
}
*/
void FotoWall::slotActionSelectAll()
{
    m_desk->selectAllContent();
}

void FotoWall::slotArrangeForceField(bool checked)
{
    m_desk->setForceFieldEnabled(checked);
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

void FotoWall::slotHelpBlog()
{
    int answer = QMessageBox::question(this, tr("Opening FotoWall's author Blog"), tr("This is the blog of the main author of FotoWall.\nYou can find some news while we set up a proper website ;-)\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QDesktopServices::openUrl(ENRICOBLOG_URL);
}

void FotoWall::slotHelpIntroduction()
{
    m_desk->showIntroduction();
}

void FotoWall::slotHelpSupport()
{
}

void FotoWall::slotHelpTutorial()
{
    int answer = QMessageBox::question(this, tr("Opening the Web Tutorial"), tr("The Tutorial is provided on Fosswire by Peter Upfold.\nIt's about FotoWall 0.2 a rather old version.\nDo you want to open the web page?"), QMessageBox::Yes, QMessageBox::No);
    if (answer == QMessageBox::Yes)
        QDesktopServices::openUrl(TUTORIAL_URL);
}


void FotoWall::slotHelpUpdates()
{
    VersionCheckDialog vcd;
    vcd.exec();
}

void FotoWall::slotSetBackMode(QAction* action)
{
    int choice = action->property("id").toUInt();
    m_desk->setBackMode(choice);
}

void FotoWall::slotBackModeChanged()
{
    int mode = m_desk->backMode();
    m_gBackActions->actions()[mode - 1]->setChecked(true);
    m_gBackActions->actions()[2]->setEnabled(mode == 3);
}

void FotoWall::slotShowPropertiesWidget(QWidget * widget)
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

void FotoWall::slotVerifyTutorial(QNetworkReply * reply)
{
    if (reply->error() != QNetworkReply::NoError)
        return;

    QString htmlCode = reply->readAll();
    bool tutorialValid = htmlCode.contains(TUTORIAL_STRING, Qt::CaseInsensitive);
    m_aHelpTutorial->setVisible(tutorialValid);
}

void FotoWall::slotVerifySupport(/*const KnowledgeItemV1List & items*/)
{
    int supportEntries = 0;
    m_aHelpSupport->setVisible(supportEntries > 0);
    m_aHelpSupport->setText(tr("Support (%1)").arg(supportEntries));
/*
    qWarning("FotoWall::slotOcsKbItems: got %d items", items.size());
    foreach (KnowledgeItemV1 * item, items) {
        qWarning() << item->name() << item->description() << item->answer();
    }
*/
}

void FotoWall::slotVerifyVideoInputs(int count)
{
    // maybe blink or something?
    ui->aAddVideo->setVisible(count > 0);
}

