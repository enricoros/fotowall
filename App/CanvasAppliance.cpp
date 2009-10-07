/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CanvasAppliance.h"

#include "Canvas/CanvasModeInfo.h"
#include "Canvas/Canvas.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/VideoProvider.h"
#include "App.h"
#include "ExactSizeDialog.h"
#include "ExportWizard.h"
#include "FotowallFile.h"
#include "Settings.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>


CanvasAppliance::CanvasAppliance(Canvas * extCanvas, int sDpiX, int sDpiY, QObject * parent)
  : Appliance::AbstractAppliance(parent)
  , m_extCanvas(extCanvas)
  , m_dummyWidget(new QWidget)
  , m_gBackActions(0)
  , m_gBackRatioActions(0)
{
    // init UI
    ui.setupUi(m_dummyWidget);
    ui.b1->setDefaultAction(ui.aAddPicture);
    ui.b2->setDefaultAction(ui.aAddText);
    ui.b3->setDefaultAction(ui.aAddWebcam);
    ui.b4->setDefaultAction(ui.aAddWordCloud);
    ui.b5->setDefaultAction(ui.aAddCanvas);
    ui.b6->setDefaultAction(ui.aSearchPictures);
    connect(ui.aAddPicture, SIGNAL(triggered()), this, SLOT(slotAddPicture()));
    connect(ui.aAddText, SIGNAL(triggered()), this, SLOT(slotAddText()));
    connect(ui.aAddWebcam, SIGNAL(triggered()), this, SLOT(slotAddWebcam()));
    connect(ui.aAddCanvas, SIGNAL(triggered()), this, SLOT(slotAddCanvas()));
    connect(ui.aAddWordCloud, SIGNAL(triggered()), this, SLOT(slotAddWordCloud()));
    connect(ui.aSearchPictures, SIGNAL(toggled(bool)), this, SLOT(slotSearchPicturesToggled(bool)));
    ui.propertiesBox->collapse();
    ui.canvasPropertiesBox->expand();
    connect(ui.projectType, SIGNAL(activated(int)), this, SLOT(slotProjectTypeActivated(int)));

    // configure the appliance
    sceneSet(m_extCanvas);
    topbarAddWidget(ui.addContentBox);
    topbarAddWidget(ui.propertiesBox);
    topbarAddWidget(ui.canvasPropertiesBox);

    // populate menus
    ui.arrangeButton->setMenu(createArrangeMenu());
    ui.backButton->setMenu(createBackgroundMenu());
    ui.decoButton->setMenu(createDecorationMenu());

    // react to canvas
    m_extCanvas->modeInfo()->setScreenDpi(sDpiX, sDpiY);
    m_extCanvas->modeInfo()->setPrintDpi(300);
    connect(m_extCanvas, SIGNAL(refreshCanvas()), this, SLOT(slotRefreshCanvas()));
    connect(m_extCanvas, SIGNAL(backModeChanged()), this, SLOT(slotBackModeChanged()));
    connect(m_extCanvas, SIGNAL(showPropertiesWidget(QWidget*)), this, SLOT(slotShowPropertiesWidget(QWidget*)));

    // react to VideoProvider
    ui.aAddWebcam->setVisible(VideoProvider::instance()->inputCount() > 0);
    connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVerifyVideoInputs(int)));

    // set the startup project mode
    int projectModeIndex = extCanvas->modeInfo()->projectMode();
    ui.projectType->setCurrentIndex(projectModeIndex);
    slotProjectTypeActivated(projectModeIndex);
}

CanvasAppliance::~CanvasAppliance()
{
    if (m_extCanvas)
        qWarning("CanvasAppliance::~CanvasAppliance: we still have a Canvas. take it before deleting this");
    delete m_dummyWidget;
}

Canvas * CanvasAppliance::takeCanvas()
{
    Canvas * canvas = m_extCanvas;
    m_extCanvas = 0;
    return canvas;
}

Canvas * CanvasAppliance::canvas() const
{
    return m_extCanvas;
}

bool CanvasAppliance::applianceCommand(int command)
{
    switch (command) {
        // Export/Print
        case App::AC_Export:
            if (m_extCanvas->modeInfo()->projectMode() == CanvasModeInfo::ModeNormal)
                return ExportWizard(m_extCanvas).exec();
            return m_extCanvas->printAsImage(m_extCanvas->modeInfo()->printDpi(), m_extCanvas->modeInfo()->fixedPrinterPixels(), m_extCanvas->modeInfo()->printLandscape());

        // Save
        case App::AC_Save: {
            // make up the default save path (stored as 'Fotowall/SaveProjectDir')
            QString defaultSavePath = tr("Unnamed %1.fotowall").arg(QDate::currentDate().toString());
            if (App::settings->contains("Fotowall/SaveProjectDir"))
                defaultSavePath.prepend(App::settings->value("Fotowall/SaveProjectDir").toString() + QDir::separator());

            // ask the file name, validate it, store back to settings and save over it
            QString fileName = QFileDialog::getSaveFileName(0, tr("Select the Fotowall file"), defaultSavePath, "Fotowall (*.fotowall)");
            if (fileName.isNull())
                return false;
            App::settings->setValue("Fotowall/SaveProjectDir", QFileInfo(fileName).absolutePath());
            if (!fileName.endsWith(".fotowall", Qt::CaseInsensitive))
                fileName += ".fotowall";
            return FotowallFile::saveV2(fileName, m_extCanvas);}

        // No Background
        case App::AC_ClearBackground:
            if (m_extCanvas->backMode() != 1) {
                ButtonsDialog query("SwitchTransparent", tr("Transparency"), tr("Now Fotowall's window is transparent.<br>Do you want me to set a transparent Canvas background too?"), QDialogButtonBox::Yes | QDialogButtonBox::No, true, true);
                query.setButtonText(QDialogButtonBox::Yes, tr("Yes, thanks"));
                query.setIcon(QStyle::SP_MessageBoxQuestion);
                if (query.execute() == QDialogButtonBox::Yes)
                    m_extCanvas->setBackMode(1);
            }
            return true;

        // Show Introduction
        case App::AC_ShowIntro:
            m_extCanvas->showIntroduction();
            return true;
    }

    // unimplemented command
    qWarning("CanvasAppliance::applianceCommand: unimplemented 0x%x", command);
    return false;
}

QMenu * CanvasAppliance::createArrangeMenu()
{
    QMenu * menu = new QMenu(m_dummyWidget);

    QAction * aForceField = new QAction(tr("Enable force field"), menu);
    aForceField->setCheckable(true);
    aForceField->setChecked(m_extCanvas->forceFieldEnabled());
    connect(aForceField, SIGNAL(toggled(bool)), this, SLOT(slotArrangeForceField(bool)));
    menu->addAction(aForceField);

    QAction * aNP = new QAction(tr("Auto-arrange new pictures"), menu);
    aNP->setCheckable(true);
    aNP->setChecked(false);
    //connect(aNP, SIGNAL(toggled(bool)), this, SLOT(slotArrangeNew(bool)));
    menu->addAction(aNP);

    menu->addSeparator()->setText(tr("Rearrange"));

    QAction * aAU = new QAction(tr("Random"), menu);
    connect(aAU, SIGNAL(triggered()), this, SLOT(slotArrangeRandom()));
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

QMenu * CanvasAppliance::createBackgroundMenu()
{
    QMenu * menu = new QMenu(m_dummyWidget);
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

QMenu * CanvasAppliance::createDecorationMenu()
{
    QMenu * menu = new QMenu(m_dummyWidget);

    QAction * aTop = new QAction(tr("Top bar"), menu);
    aTop->setCheckable(true);
    aTop->setChecked(m_extCanvas->topBarEnabled());
    connect(aTop, SIGNAL(toggled(bool)), this, SLOT(slotDecoTopBar(bool)));
    menu->addAction(aTop);

    QAction * aBottom = new QAction(tr("Bottom bar"), menu);
    aBottom->setCheckable(true);
    aBottom->setChecked(m_extCanvas->bottomBarEnabled());
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

void CanvasAppliance::setNormalProject()
{
    m_extCanvas->modeInfo()->setFixedSizeInches();
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeNormal);
    m_extCanvas->clearMarkers();
    containerValueSet(App::CV_ExPrint, false);
    ui.projectType->setCurrentIndex(0);
}

void CanvasAppliance::setCDProject()
{
    m_extCanvas->clearMarkers();
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(4.75, 4.75));
    m_extCanvas->modeInfo()->setPrintLandscape(false);
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeCD);
    containerValueSet(App::CV_ExPrint, true);
    ui.projectType->setCurrentIndex(1);
}

void CanvasAppliance::setDVDProject()
{
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(10.83, 7.2));
    m_extCanvas->modeInfo()->setPrintLandscape(true);
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeDVD);
    containerValueSet(App::CV_ExPrint, true);
    ui.projectType->setCurrentIndex(2);
    m_extCanvas->setDVDMarkers();
}

void CanvasAppliance::setExactSizeProject()
{
    m_extCanvas->clearMarkers();
    if (!m_extCanvas->modeInfo()->fixedSize()) {
        ExactSizeDialog sizeDialog;
        QPointF screenDpi = m_extCanvas->modeInfo()->screenDpi();
        if (screenDpi.x() == screenDpi.y())
            sizeDialog.ui.screenDpi->setValue(screenDpi.x());
        else
            sizeDialog.ui.screenDpi->setSpecialValueText(QString("%1, %2").arg(screenDpi.x()).arg(screenDpi.y()));
        if (sizeDialog.exec() != QDialog::Accepted)
            return;
        float w = sizeDialog.ui.widthSpinBox->value();
        float h = sizeDialog.ui.heightSpinBox->value();
        int printDpi = sizeDialog.ui.printDpi->value();
        bool landscape = sizeDialog.ui.landscapeCheckBox->isChecked();
        bool cm = !sizeDialog.ui.unityComboBox->currentIndex();
        m_extCanvas->modeInfo()->setPrintLandscape(landscape);
        m_extCanvas->modeInfo()->setPrintDpi(printDpi);
        m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(cm?(double)w/2.54:w, cm?(double)h/2.54:h));
    }
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeExactSize);
    containerValueSet(App::CV_ExPrint, true);
    ui.projectType->setCurrentIndex(3);
}


void CanvasAppliance::slotAddCanvas()
{
    // make up the default load path (stored as 'Fotowall/LoadProjectDir')
    QString defaultLoadPath = App::settings->value("Fotowall/LoadProjectDir").toString();

    // ask the file name, validate it, store back to settings and load the file
    QStringList fileNames = QFileDialog::getOpenFileNames(0, tr("Select one or more Fotowall files to add"), defaultLoadPath, tr("Fotowall (*.fotowall)"));
    if (fileNames.isEmpty())
        return;
    App::settings->setValue("Fotowall/LoadProjectDir", QFileInfo(fileNames[0]).absolutePath());
    m_extCanvas->addCanvasViewContent(fileNames);
}

void CanvasAppliance::slotAddPicture()
{
    // make up the default load path (stored as 'Fotowall/LoadImagesDir')
    QString defaultLoadPath = App::settings->value("Fotowall/LoadImagesDir").toString();

    // ask the file name, validate it, store back to settings and load the file
    QStringList fileNames = QFileDialog::getOpenFileNames(0, tr("Select one or more pictures to add"), defaultLoadPath, tr("Images (%1)").arg(App::supportedImageFormats()));
    if (fileNames.isEmpty())
        return;
    App::settings->setValue("Fotowall/LoadImagesDir", QFileInfo(fileNames[0]).absolutePath());
    m_extCanvas->addPictureContent(fileNames);
}

void CanvasAppliance::slotAddText()
{
    m_extCanvas->addTextContent();
}

void CanvasAppliance::slotAddWebcam()
{
    m_extCanvas->addWebcamContent(0);
}

void CanvasAppliance::slotAddWordCloud()
{
    m_extCanvas->addWordCloudContent();
}

void CanvasAppliance::slotSearchPicturesToggled(bool visible)
{
    m_extCanvas->setSearchPicturesVisible(visible);
}


void CanvasAppliance::slotProjectTypeActivated(int index)
{
    switch (index) {
        case 0: setNormalProject();     break;
        case 1: setCDProject();         break;
        case 2: setDVDProject();        break;
        case 3: m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF());
                setExactSizeProject();  break;
    }
    containerValueSet(App::CV_RefreshScene, true);
}

void CanvasAppliance::slotSetBackMode(QAction* action)
{
    int choice = action->property("id").toUInt();
    m_extCanvas->setBackMode(choice);
}

void CanvasAppliance::slotSetBackRatio(QAction* action)
{
    Qt::AspectRatioMode mode = (Qt::AspectRatioMode)action->property("mode").toInt();
    m_extCanvas->setBackContentRatio(mode);
}

void CanvasAppliance::slotArrangeForceField(bool checked)
{
    m_extCanvas->setForceFieldEnabled(checked);
}

#include "Canvas/AbstractContent.h"
void CanvasAppliance::slotArrangeRandom()
{
    QRectF r = m_extCanvas->sceneRect();
    foreach (QGraphicsItem * item, m_extCanvas->items()) {
        AbstractContent * content = dynamic_cast<AbstractContent *>(item);
        if (!content)
            continue;
        content->setPos(r.left() + (qrand() % (int)r.width()), r.top() + (qrand() % (int)r.height()));
        content->setRotation(-30 + (qrand() % 60));
#if QT_VERSION >= 0x040500
        content->setOpacity((qreal)(qrand() % 100) / 99.0);
#endif
    }
}

void CanvasAppliance::slotDecoTopBar(bool checked)
{
    m_extCanvas->setTopBarEnabled(checked);
}

void CanvasAppliance::slotDecoBottomBar(bool checked)
{
    m_extCanvas->setBottomBarEnabled(checked);
}

void CanvasAppliance::slotDecoSetTitle()
{
    // set a dummy title, if none
    if (m_extCanvas->titleText().isEmpty())
        m_extCanvas->setTitleText("...");

    // change title dialog
    bool ok = false;
    QString title = QInputDialog::getText(0, tr("Title"), tr("Insert the title"), QLineEdit::Normal, m_extCanvas->titleText(), &ok);
    if (ok)
        m_extCanvas->setTitleText(title);
}

void CanvasAppliance::slotDecoClearTitle()
{
    m_extCanvas->setTitleText(QString());
}

void CanvasAppliance::slotRefreshCanvas()
{
    int modeIndex = m_extCanvas->modeInfo()->projectMode();
    // called here not to have the unneeded size dialog
    if (modeIndex == CanvasModeInfo::ModeExactSize)
        setExactSizeProject();
    else
        ui.projectType->setCurrentIndex(modeIndex);
}

void CanvasAppliance::slotBackModeChanged()
{
    int mode = m_extCanvas->backMode();
    m_gBackActions->actions()[mode - 1]->setChecked(true);
    m_gBackActions->actions()[2]->setEnabled(mode == 3);
}

void CanvasAppliance::slotBackRatioChanged()
{
    Qt::AspectRatioMode mode = m_extCanvas->backContentRatio();
    if (mode == Qt::KeepAspectRatioByExpanding)
        m_gBackRatioActions->actions()[0]->setChecked(true);
    else if (mode == Qt::KeepAspectRatio)
        m_gBackRatioActions->actions()[1]->setChecked(true);
    else if (mode == Qt::IgnoreAspectRatio)
        m_gBackRatioActions->actions()[2]->setChecked(true);
}


void CanvasAppliance::slotShowPropertiesWidget(QWidget * widget)
{
    // delete current Properties content
    QLayoutItem * prevItem = ui.propLayout->takeAt(0);
    if (prevItem) {
        delete prevItem->widget();
        delete prevItem;
    }

    // show the Properties container with new content and title
    if (widget) {
        ui.canvasPropertiesBox->collapse();
        widget->setParent(ui.propertiesBox);
        ui.propLayout->addWidget(widget);
        ui.propertiesBox->setTitle(widget->windowTitle());
        ui.propertiesBox->expand();
    }
    // or show the Canvas container
    else {
        ui.propertiesBox->collapse();
        ui.canvasPropertiesBox->expand();
    }
}

void CanvasAppliance::slotVerifyVideoInputs(int count)
{
    // maybe blink or something?
    ui.aAddWebcam->setVisible(count > 0);
}
