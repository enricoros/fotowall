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

#include "Shared/CommandStack.h"
#include "Shared/Commands.h"

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


CanvasAppliance::CanvasAppliance(Canvas * extCanvas, int sDpiX, int sDpiY, QObject * parent)
  : QObject(parent)
  , m_extCanvas(extCanvas)
  , m_dummyWidget(new QWidget)
  , m_gBackModeGroup(0)
  , m_gBackRatioGroup(0)
  , m_gBackContentAction(0)
{
    // init UI
    ui.setupUi(m_dummyWidget);
    ui.b1->setDefaultAction(ui.aAddPicture);
    ui.b2->setDefaultAction(ui.aAddText);
    ui.b3->setDefaultAction(ui.aAddWebcam);
    ui.b4->setDefaultAction(ui.aAddWordcloud);
    ui.b5->setDefaultAction(ui.aAddCanvas);
    ui.b6->setDefaultAction(ui.aSearchPictures);
    connect(ui.aAddPicture, SIGNAL(triggered()), this, SLOT(slotAddPicture()));
    connect(ui.aAddText, SIGNAL(triggered()), this, SLOT(slotAddText()));
    connect(ui.aAddWebcam, SIGNAL(triggered()), this, SLOT(slotAddWebcam()));
    connect(ui.aAddCanvas, SIGNAL(triggered()), this, SLOT(slotAddCanvas()));
    connect(ui.aAddWordcloud, SIGNAL(triggered()), this, SLOT(slotAddWordcloud()));
    connect(ui.aSearchPictures, SIGNAL(toggled(bool)), this, SLOT(slotSearchPicturesToggled(bool)));
    ui.propertiesBox->collapse();
    ui.canvasPropertiesBox->expand();
    connect(ui.projectCombo, SIGNAL(activated(int)), this, SLOT(slotProjectComboActivated(int)));
    connect(ui.loadButton, SIGNAL(clicked()), this, SLOT(slotFileLoad()));
    connect(ui.saveButton, SIGNAL(clicked()), this, SLOT(slotFileSave()));
    connect(ui.exportButton, SIGNAL(clicked()), this, SLOT(slotFileExport()));

    // configure the appliance
    sceneSet(m_extCanvas);
    topbarAddWidget(ui.addContentBox);
    topbarAddWidget(ui.propertiesBox);
    topbarAddWidget(ui.canvasPropertiesBox);
    topbarAddWidget(ui.fileWidget, true);

    // populate menus
    ui.arrangeButton->setMenu(createArrangeMenu());
    ui.backButton->setMenu(createBackgroundMenu());
    ui.decoButton->setMenu(createDecorationMenu());

    // react to canvas
    m_extCanvas->modeInfo()->setScreenDpi(sDpiX, sDpiY);
    m_extCanvas->modeInfo()->setPrintDpi(300);
    connect(m_extCanvas, SIGNAL(backConfigChanged()), this, SLOT(slotBackConfigChanged()));
    connect(m_extCanvas, SIGNAL(requestContentEditing(AbstractContent*)), this, SLOT(slotEditContent(AbstractContent*)));
    connect(m_extCanvas, SIGNAL(showPropertiesWidget(QWidget*)), this, SLOT(slotShowPropertiesWidget(QWidget*)));

    // react to VideoProvider
    ui.aAddWebcam->setVisible(VideoProvider::instance()->inputCount() > 0);
    connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVerifyVideoInputs(int)));

    // set the startup project mode
    int projectModeIndex = extCanvas->modeInfo()->projectMode();
    slotProjectComboActivated(projectModeIndex);
}

CanvasAppliance::~CanvasAppliance()
{
    if (m_extCanvas)
        qWarning("CanvasAppliance::~CanvasAppliance: we still have a Canvas. take it before deleting this");
    delete ui.addContentBox;
    delete ui.propertiesBox;
    delete ui.canvasPropertiesBox;
    delete ui.fileWidget;
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
        QString fileName = FotowallFile::getSaveFotowallFile();
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

void CanvasAppliance::setCDProject()
{
    ui.projectCombo->setCurrentIndex(1);
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(4.75, 4.75));
    m_extCanvas->modeInfo()->setPrintLandscape(false);
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeCD);
    m_extCanvas->adjustSceneSize();
    m_extCanvas->setCDMarkers();
    configurePrint(true);
}

void CanvasAppliance::setDVDProject()
{
    ui.projectCombo->setCurrentIndex(2);
    m_extCanvas->modeInfo()->setFixedSizeInches(QSizeF(10.83, 7.2));
    m_extCanvas->modeInfo()->setPrintLandscape(true);
    m_extCanvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeDVD);
    m_extCanvas->adjustSceneSize();
    m_extCanvas->setDVDMarkers();
    configurePrint(true);
}

void CanvasAppliance::setExactSizeProject(bool usePrevious)
{
    ui.projectCombo->setCurrentIndex(3);
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

void CanvasAppliance::configurePrint(bool enabled)
{
    ui.exportButton->setText(enabled ? tr("Print") : tr("Export"));
}

void CanvasAppliance::slotAddCanvas()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);

    QStringList fileNames = FotowallFile::getLoadFotowallFiles();
    if (fileNames.isEmpty())
        return;
    m_extCanvas->addCanvasViewContent(fileNames);
}

void CanvasAppliance::slotAddPicture()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);

    // make up the default load path (stored as 'Fotowall/LoadImagesDir')
    QString defaultLoadPath = App::settings->value("Fotowall/LoadImagesDir").toString();

    // ask the file name, validate it, store back to settings and load the file
    QStringList fileNames = QFileDialog::getOpenFileNames(0, tr("Select one or more pictures to add"), defaultLoadPath, tr("Images (%1)").arg(App::supportedImageFormats()));
    if (fileNames.isEmpty())
        return;
    App::settings->setValue("Fotowall/LoadImagesDir", QFileInfo(fileNames[0]).absolutePath());
    NewImageCommand *c = new NewImageCommand(m_extCanvas, fileNames);
    CommandStack::instance().doCommand(c);
}

void CanvasAppliance::slotAddText()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);
    NewTextCommand *c = new NewTextCommand(m_extCanvas);
    CommandStack::instance().doCommand(c);
}

void CanvasAppliance::slotAddWebcam()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);
    NewWebcamCommand *c = new NewWebcamCommand(m_extCanvas);
    CommandStack::instance().doCommand(c);
}

void CanvasAppliance::slotAddWordcloud()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);
    m_extCanvas->addWordcloudContent();
}

void CanvasAppliance::slotSearchPicturesToggled(bool visible)
{
    m_extCanvas->setSearchPicturesVisible(visible);
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
        case 1: setCDProject();                 break;
        case 2: setDVDProject();                break;
        case 3: setExactSizeProject(!sender()); break;
    }
}

void CanvasAppliance::slotSetBackMode(QAction* action)
{
    Canvas::BackMode mode = (Canvas::BackMode)action->property("modeId").toInt();
    BackgroundModeCommand *command = new BackgroundModeCommand(m_extCanvas, m_extCanvas->backMode(), mode);
    CommandStack::instance().doCommand(command);
}

void CanvasAppliance::slotSetBackRatio(QAction* action)
{
    Qt::AspectRatioMode ratio = (Qt::AspectRatioMode)action->property("ratioId").toInt();
    BackgroundRatioCommand *command = new BackgroundRatioCommand(m_extCanvas, m_extCanvas->backContentRatio(), ratio);
    CommandStack::instance().doCommand(command);
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
    DecoTopBarCommand *c = new DecoTopBarCommand(m_extCanvas, checked);
    CommandStack::instance().doCommand(c);
}

void CanvasAppliance::slotDecoBottomBar(bool checked)
{
    DecoBottomBarCommand *c = new DecoBottomBarCommand(m_extCanvas, checked);
    CommandStack::instance().doCommand(c);
}

void CanvasAppliance::slotDecoSetTitle()
{
    // change title dialog
    bool ok = false;
    QString title = QInputDialog::getText(0, tr("Title"), tr("Insert the title"), QLineEdit::Normal, m_extCanvas->titleText(), &ok);
    if (ok) {
        DecoTitleCommand *c = new DecoTitleCommand(m_extCanvas, title);
        CommandStack::instance().doCommand(c);
    }

    // set a dummy title, if none
    if (m_extCanvas->titleText().isEmpty())
        m_extCanvas->setTitleText("...");

}

void CanvasAppliance::slotDecoClearTitle()
{
    DecoTitleCommand *c = new DecoTitleCommand(m_extCanvas, QString());
    CommandStack::instance().doCommand(c);
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
        App::workflow->stackSlaveWordcloud_A(wc);
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
