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
#include "Canvas/CanvasViewContent.h"
#include "Canvas/Canvas.h"
#include "Canvas/WordcloudContent.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/VideoProvider.h"
#include "App.h"
#include "ExactSizeDialog.h"
#include "ExportWizard.h"
#include "FotowallFile.h"
#include "Settings.h"
#include "Workflow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>


CanvasAppliance::CanvasAppliance(Canvas * extCanvas, QObject * parent)
  : QObject(parent)
  , m_extCanvas(extCanvas)
  , m_dummyWidget(new QWidget)
  , m_gBackModeGroup(0)
  , m_gBackRatioGroup(0)
  , m_gBackContentAction(0)
{
    // init UI
    ui.setupUi(m_dummyWidget);
    ui.addContentBox->setFixedHeight(App::TopBarHeight);
    ui.propertiesBox->setFixedHeight(App::TopBarHeight);
    ui.canvasPropertiesBox->setFixedHeight(App::TopBarHeight);
    ui.fileBox->setFixedHeight(App::TopBarHeight);
    connect(ui.bPicture, SIGNAL(clicked()), this, SLOT(slotAddPicture()));
    connect(ui.bText, SIGNAL(clicked()), this, SLOT(slotAddText()));
    connect(ui.bWebcam, SIGNAL(clicked()), this, SLOT(slotAddWebcam()));
    connect(ui.bWordcloud, SIGNAL(clicked()), this, SLOT(slotAddWordcloud()));
    connect(ui.bCanvas, SIGNAL(clicked()), this, SLOT(slotAddCanvas()));
    connect(ui.bWebsearch, SIGNAL(toggled(bool)), this, SLOT(slotSearchPicturesToggled(bool)));
    ui.propertiesBox->collapse();
    ui.canvasPropertiesBox->expand();
    connect(ui.projectCombo, SIGNAL(activated(int)), this, SLOT(slotProjectComboActivated(int)));
    connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(slotFileSave()));
    connect(ui.exportButton, SIGNAL(clicked()), this, SLOT(slotFileExport()));

    // configure the appliance
    windowTitleSet(m_extCanvas->prettyBaseName());
    sceneSet(m_extCanvas);
    topbarAddWidget(ui.addContentBox);
    topbarAddWidget(ui.propertiesBox);
    topbarAddWidget(ui.canvasPropertiesBox);
    topbarAddWidget(ui.fileBox, true);

    // populate menus
    ui.arrangeButton->setMenu(createArrangeMenu());
    ui.backButton->setMenu(createBackgroundMenu());
    ui.decoButton->setMenu(createDecorationMenu());

    // react to canvas
    connect(m_extCanvas, SIGNAL(backConfigChanged()), this, SLOT(slotBackConfigChanged()));
    connect(m_extCanvas, SIGNAL(requestContentEditing(AbstractContent*)), this, SLOT(slotEditContent(AbstractContent*)));
    connect(m_extCanvas, SIGNAL(showPropertiesWidget(QWidget*)), this, SLOT(slotShowPropertiesWidget(QWidget*)));
    connect(m_extCanvas, SIGNAL(filePathChanged()), this, SLOT(slotFilePathChanged()));

    // react to VideoProvider
    ui.bWebcam->setVisible(VideoProvider::instance()->inputCount() > 0);
    connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVerifyVideoInputs(int)));

    // set the startup project mode
    int pComboIndex = 0;
    switch (extCanvas->modeInfo()->projectMode()) {
        case CanvasModeInfo::ModeNormal:    pComboIndex = 0; break;
        case CanvasModeInfo::ModeCD:        pComboIndex = 2; break;
        case CanvasModeInfo::ModeDVD:       pComboIndex = 3; break;
        case CanvasModeInfo::ModeExactSize: pComboIndex = 1; break;
    }
    slotProjectComboActivated(pComboIndex);
}

CanvasAppliance::~CanvasAppliance()
{
    if (m_extCanvas)
        qWarning("CanvasAppliance::~CanvasAppliance: we still have a Canvas. take it before deleting this");
    delete ui.addContentBox;
    delete ui.propertiesBox;
    delete ui.canvasPropertiesBox;
    delete ui.fileBox;
    delete m_dummyWidget;
}

Canvas * CanvasAppliance::takeCanvas()
{
    Canvas * canvas = m_extCanvas;
    disconnect(canvas, 0, this, 0);
    m_extCanvas = 0;
    sceneClear();
    return canvas;
}

bool CanvasAppliance::pendingChanges() const
{
    return m_extCanvas ? m_extCanvas->pendingChanges() : false;
}

bool CanvasAppliance::saveToFile(const QString & __fileName)
{
    // ask for file name if not given
    if (__fileName.isEmpty()) {
        QString fileName = FotowallFile::getSaveFotowallFile(m_extCanvas->filePath());
        if (fileName.isNull())
            return false;
        return FotowallFile::saveV2(fileName, m_extCanvas);
    }

    return FotowallFile::saveV2(__fileName, m_extCanvas);
}

bool CanvasAppliance::applianceCommand(int command)
{
    switch (command) {
        // No Background
        case App::AC_ClearBackground:
            if (m_extCanvas->backMode() != Canvas::BackNone) {
                ButtonsDialog query("SwitchTransparent", tr("Transparency"), tr("You won't see through the Canvas unless you remove the background gradient.<br><b>Do you want me to clear the background?</b>"), QDialogButtonBox::Yes | QDialogButtonBox::No, true, true);
                query.setButtonText(QDialogButtonBox::Yes, tr("Yes, thanks"));
                query.setIcon(QStyle::SP_MessageBoxQuestion);
                if (query.execute() == QDialogButtonBox::Yes)
                    m_extCanvas->setBackMode(Canvas::BackNone);
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
    QAction * aAU = new QAction(tr("Random Placement"), menu);
     aAU->setShortcut(QKeySequence("CTRL+SHIFT+R"));
     connect(aAU, SIGNAL(triggered()), this, SLOT(slotArrangeRandom()));
     menu->addAction(aAU);
    QAction * aAS = new QAction(tr("Shaped Placement"), menu);
     aAS->setShortcut(QKeySequence("CTRL+SHIFT+S"));
     aAS->setEnabled(false);
     connect(aAS, SIGNAL(triggered()), this, SLOT(slotArrangeShaped()));
     menu->addAction(aAS);
    QAction * aAC = new QAction(tr("Color Collage"), menu);
     aAC->setShortcut(QKeySequence("CTRL+SHIFT+C"));
     aAC->setEnabled(false);
     connect(aAC, SIGNAL(triggered()), this, SLOT(slotArrangeColorCollage()));
     menu->addAction(aAC);
    menu->addSeparator();
    QAction * aForceField = new QAction(tr("Enable force field"), menu);
     aForceField->setShortcut(QKeySequence("CTRL+SHIFT+F"));
     aForceField->setCheckable(true);
     aForceField->setChecked(m_extCanvas->forceFieldEnabled());
     connect(aForceField, SIGNAL(toggled(bool)), this, SLOT(slotArrangeForceField(bool)));
     menu->addAction(aForceField);
    return menu;
}

QMenu * CanvasAppliance::createBackgroundMenu()
{
    QMenu * menu = new QMenu(m_dummyWidget);
    m_gBackModeGroup = new QActionGroup(menu);
    connect(m_gBackModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotSetBackMode(QAction*)));
    QAction * aNone = new QAction(tr("None"), menu);
     aNone->setToolTip(tr("Transparency can be saved to PNG images only."));
     aNone->setShortcut(QKeySequence("CTRL+1"));
     aNone->setProperty("modeId", (int)Canvas::BackNone);
     aNone->setCheckable(true);
     aNone->setActionGroup(m_gBackModeGroup);
     menu->addAction(aNone);
    QAction * aBlack = new QAction(tr("Black"), menu);
     aBlack->setShortcut(QKeySequence("CTRL+2"));
     aBlack->setProperty("modeId", (int)Canvas::BackBlack);
     aBlack->setCheckable(true);
     aBlack->setActionGroup(m_gBackModeGroup);
     menu->addAction(aBlack);
    QAction * aWhite = new QAction(tr("White"), menu);
     aWhite->setShortcut(QKeySequence("CTRL+3"));
     aWhite->setProperty("modeId", (int)Canvas::BackWhite);
     aWhite->setCheckable(true);
     aWhite->setActionGroup(m_gBackModeGroup);
     menu->addAction(aWhite);
    QAction * aGradient = new QAction(tr("Gradient"), menu);
     aGradient->setShortcut(QKeySequence("CTRL+4"));
     aGradient->setProperty("modeId", (int)Canvas::BackGradient);
     aGradient->setCheckable(true);
     aGradient->setActionGroup(m_gBackModeGroup);
     menu->addAction(aGradient);
    menu->addSeparator();
    m_gBackContentAction = new QAction(tr("Content"), menu);
     connect(m_gBackContentAction, SIGNAL(toggled(bool)), this, SLOT(slotBackContentRemove(bool)));
     m_gBackContentAction->setToolTip(tr("Double click on any content to put it on background."));
     m_gBackContentAction->setEnabled(false);
     m_gBackContentAction->setCheckable(true);
     menu->addAction(m_gBackContentAction);
    QMenu * mScaling = new QMenu(tr("Content Aspect Ratio"), menu);
     m_gBackRatioGroup = new QActionGroup(menu);
     connect(m_gBackRatioGroup, SIGNAL(triggered(QAction*)), this, SLOT(slotSetBackRatio(QAction*)));
     menu->addMenu(mScaling);
    QAction * aRatioKeepEx = new QAction(tr("Keep proportions by expanding"), mScaling);
     aRatioKeepEx->setShortcut(QKeySequence("CTRL+5"));
     aRatioKeepEx->setProperty("ratioId", (int)Qt::KeepAspectRatioByExpanding);
     aRatioKeepEx->setCheckable(true);
     aRatioKeepEx->setActionGroup(m_gBackRatioGroup);
     mScaling->addAction(aRatioKeepEx);
    QAction * aRatioKeep = new QAction(tr("Keep proportions"), mScaling);
     aRatioKeep->setShortcut(QKeySequence("CTRL+6"));
     aRatioKeep->setProperty("ratioId", (int)Qt::KeepAspectRatio);
     aRatioKeep->setCheckable(true);
     aRatioKeep->setActionGroup(m_gBackRatioGroup);
     mScaling->addAction(aRatioKeep);
    QAction * aRatioIgnore = new QAction(tr("Ignore proportions"), mScaling);
     aRatioIgnore->setShortcut(QKeySequence("CTRL+7"));
     aRatioIgnore->setProperty("ratioId", (int)Qt::IgnoreAspectRatio);
     aRatioIgnore->setCheckable(true);
     aRatioIgnore->setActionGroup(m_gBackRatioGroup);
     mScaling->addAction(aRatioIgnore);

    // initially check the action
    slotBackConfigChanged();
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
    ui.projectCombo->setCurrentIndex(0);
    m_extCanvas->clearMarkers();
    m_extCanvas->modeInfo()->setFixedSizeInches();
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeNormal);
    m_extCanvas->adjustSceneSize();
    configurePrint(false);
}

void CanvasAppliance::setExactSizeProject(bool usePrevious)
{
    ui.projectCombo->setCurrentIndex(1);
    m_extCanvas->clearMarkers();
    if (!usePrevious || !m_extCanvas->modeInfo()->fixedSize()) {
        ExactSizeDialog sizeDialog;
        QSizeF prevSizeInches = m_extCanvas->modeInfo()->fixedSizeInches();
        if (!prevSizeInches.isEmpty()) {
            bool cm = !sizeDialog.ui.unityComboBox->currentIndex();
            sizeDialog.ui.widthSpinBox->setValue(cm?prevSizeInches.width()*2.54:prevSizeInches.width());
            sizeDialog.ui.heightSpinBox->setValue(cm?prevSizeInches.height()*2.54:prevSizeInches.height());
        }
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
    m_extCanvas->adjustSceneSize();
    configurePrint(true);
}

void CanvasAppliance::setCDProject()
{
    ui.projectCombo->setCurrentIndex(2);
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(4.75, 4.75));
    m_extCanvas->modeInfo()->setPrintLandscape(false);
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeCD);
    m_extCanvas->adjustSceneSize();
    m_extCanvas->setCDMarkers();
    configurePrint(true);
}

void CanvasAppliance::setDVDProject()
{
    ui.projectCombo->setCurrentIndex(3);
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(10.83, 7.2));
    m_extCanvas->modeInfo()->setPrintLandscape(true);
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeDVD);
    m_extCanvas->adjustSceneSize();
    m_extCanvas->setDVDMarkers();
    configurePrint(true);
}

void CanvasAppliance::configurePrint(bool enabled)
{
    ui.exportButton->setText(enabled ? tr("Print") : tr("Export"));
}

void CanvasAppliance::slotAddCanvas()
{
    QStringList fileNames = FotowallFile::getLoadFotowallFiles();
    if (fileNames.isEmpty())
        return;
    m_extCanvas->addCanvasViewContent(fileNames);
    setFocusToScene();
}

void CanvasAppliance::slotAddPicture()
{
    // make up the default load path (stored as 'Fotowall/LoadImagesDir')
    QString defaultLoadPath = App::settings->value("Fotowall/LoadImagesDir").toString();

    // ask the file name, validate it, store back to settings and load the file
    QStringList fileNames = QFileDialog::getOpenFileNames(0, tr("Add Pictures to the Canvas"), defaultLoadPath, tr("Images (%1)").arg(App::supportedImageFormats()));
    if (fileNames.isEmpty())
        return;
    App::settings->setValue("Fotowall/LoadImagesDir", QFileInfo(fileNames[0]).absolutePath());
    m_extCanvas->addPictureContent(fileNames);
    setFocusToScene();
}

void CanvasAppliance::slotAddText()
{
    m_extCanvas->addTextContent();
    setFocusToScene();
}

void CanvasAppliance::slotAddWebcam()
{
    m_extCanvas->addWebcamContent(0);
    setFocusToScene();
}

void CanvasAppliance::slotAddWordcloud()
{
    m_extCanvas->addWordcloudContent();
    setFocusToScene();
}

void CanvasAppliance::slotSearchPicturesToggled(bool visible)
{
    containerValueSet(App::CC_ShowPictureSearch, visible);
}


void CanvasAppliance::slotBackContentRemove(bool checked)
{
    if (!checked)
        m_extCanvas->clearBackContent();
}

void CanvasAppliance::slotProjectComboActivated(int index)
{
    switch (index) {
        case 0: setNormalProject();             break;
        case 1: setExactSizeProject(!sender()); break;
        case 2: setCDProject();                 break;
        case 3: setDVDProject();                break;
    }
}

void CanvasAppliance::slotSetBackMode(QAction* action)
{
    Canvas::BackMode mode = (Canvas::BackMode)action->property("modeId").toInt();
    m_extCanvas->setBackMode(mode);
}

void CanvasAppliance::slotSetBackRatio(QAction* action)
{
    Qt::AspectRatioMode ratio = (Qt::AspectRatioMode)action->property("ratioId").toInt();
    m_extCanvas->setBackContentRatio(ratio);
}

void CanvasAppliance::slotArrangeForceField(bool checked)
{
    m_extCanvas->setForceFieldEnabled(checked);
}

void CanvasAppliance::slotArrangeColorCollage()
{
    HERE
}

void CanvasAppliance::slotArrangeRandom()
{
    m_extCanvas->randomizeContents(true, true, true);
}

void CanvasAppliance::slotArrangeShaped()
{
    HERE
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

bool CanvasAppliance::slotFileLoad()
{
    return App::workflow->loadCanvas_A();
}

bool CanvasAppliance::slotFileSave()
{
    return saveToFile();
}

bool CanvasAppliance::slotFileExport()
{
    return ExportWizard(m_extCanvas, m_extCanvas->modeInfo()->projectMode() != CanvasModeInfo::ModeNormal).exec();
}

void CanvasAppliance::slotEditContent(AbstractContent *content)
{
    // handle Canvas
    if (CanvasViewContent * cvc = dynamic_cast<CanvasViewContent *>(content)) {
        App::workflow->stackSlaveCanvas_A(cvc);
        return;
    }

    // handle Wordcloud
    if (WordcloudContent * wc = dynamic_cast<WordcloudContent *>(content)) {
#ifndef NO_WORDCLOUD_APPLIANCE
        App::workflow->stackSlaveWordcloud_A(wc);
#else
        Q_UNUSED(wc);
        ButtonsDialog info("WordcloudMissingInfo", tr("Wordcloud Editor"), tr("The Wordcloud editor will be ready in the Fotowall REVO (1.0) release."), QDialogButtonBox::Ok, true, true);
        info.execute();
#endif
        return;
    }
}

void CanvasAppliance::slotBackConfigChanged()
{
    // update ratios to reflect background mode
    m_gBackModeGroup->blockSignals(true);
    int mode = (int)m_extCanvas->backMode();
    m_gBackModeGroup->actions()[qMax(0, mode)]->setChecked(true);
    m_gBackModeGroup->blockSignals(false);

    // update check to reflect background content presence
    m_gBackContentAction->blockSignals(true);
    bool hasBackContent = m_extCanvas->backContent();
    m_gBackContentAction->setChecked(hasBackContent);
    m_gBackContentAction->setEnabled(hasBackContent);
    m_gBackContentAction->blockSignals(false);

    // update ratios to reflect content ratio
    Qt::AspectRatioMode cRatio = m_extCanvas->backContentRatio();
    if (cRatio == Qt::KeepAspectRatioByExpanding)
        m_gBackRatioGroup->actions()[0]->setChecked(true);
    else if (cRatio == Qt::KeepAspectRatio)
        m_gBackRatioGroup->actions()[1]->setChecked(true);
    else if (cRatio == Qt::IgnoreAspectRatio)
        m_gBackRatioGroup->actions()[2]->setChecked(true);
}

void CanvasAppliance::slotShowPropertiesWidget(QWidget * widget)
{
    // delete current Properties content
    QLayoutItem * prevItem = ui.propLayout->takeAt(0);
    if (prevItem) {
        delete prevItem->widget();
        delete prevItem;
    }
    if (!ui.propLayout->isEmpty())
        qWarning("CanvasAppliance::slotShowPropertiesWidget: problem in the properties layout: not empty (%d)", ui.propLayout->count());

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

void CanvasAppliance::slotFilePathChanged()
{
    windowTitleSet(m_extCanvas->prettyBaseName());
}

void CanvasAppliance::slotVerifyVideoInputs(int count)
{
    // maybe blink or something?
    ui.bWebcam->setVisible(count > 0);
}
