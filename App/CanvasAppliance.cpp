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


CanvasAppliance::CanvasAppliance(Canvas * extCanvas, int sDpiX, int sDpiY, QObject * parent)
  : QObject(parent)
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
    connect(m_extCanvas, SIGNAL(requestContentEditing(AbstractContent*)), this, SLOT(slotEditContent(AbstractContent*)));
    connect(m_extCanvas, SIGNAL(backModeChanged()), this, SLOT(slotBackModeChanged()));
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
    return canvas;
}

bool CanvasAppliance::saveToFile(const QString & __fileName)
{
    // ask for file name if not given
    if (__fileName.isEmpty()) {
        QString defaultSavePath = tr("Unnamed %1.fotowall").arg(QDate::currentDate().toString());
        if (App::settings->contains("Fotowall/SaveProjectDir"))
            defaultSavePath.prepend(App::settings->value("Fotowall/SaveProjectDir").toString() + QDir::separator());
        QString fileName = QFileDialog::getSaveFileName(0, tr("Select the Fotowall file"), defaultSavePath, "Fotowall (*.fotowall)");
        if (fileName.isNull())
            return false;
        App::settings->setValue("Fotowall/SaveProjectDir", QFileInfo(fileName).absolutePath());
        if (!fileName.endsWith(".fotowall", Qt::CaseInsensitive))
            fileName += ".fotowall";
        return FotowallFile::saveV2(fileName, m_extCanvas);
    }

    return FotowallFile::saveV2(__fileName, m_extCanvas);
}

bool CanvasAppliance::applianceCommand(int command)
{
    switch (command) {
        // No Background
        case App::AC_ClearBackground:
            if (m_extCanvas->backMode() == 2) {
                ButtonsDialog query("SwitchTransparent", tr("Transparency"), tr("You won't see through the Canvas unless you remove the background gradient.<br><b>Do you want me to clear the background?</b>"), QDialogButtonBox::Yes | QDialogButtonBox::No, true, true);
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
    m_gBackActions = new QActionGroup(menu);
    connect(m_gBackActions, SIGNAL(triggered(QAction*)), this, SLOT(slotSetBackMode(QAction*)));
    QAction * aNone = new QAction(tr("None"), menu);
     aNone->setToolTip(tr("Transparency can be saved to PNG images only."));
     aNone->setShortcut(QKeySequence("CTRL+1"));
     aNone->setProperty("id", 1);
     aNone->setCheckable(true);
     aNone->setActionGroup(m_gBackActions);
     menu->addAction(aNone);
    QAction * aGradient = new QAction(tr("Gradient"), menu);
     aGradient->setShortcut(QKeySequence("CTRL+2"));
     aGradient->setProperty("id", 2);
     aGradient->setCheckable(true);
     aGradient->setActionGroup(m_gBackActions);
     menu->addAction(aGradient);
    QAction * aContent = new QAction(tr("Content"), menu);
     aContent->setToolTip(tr("Double click on any content to put it on background."));
     aContent->setShortcut(QKeySequence("CTRL+3"));
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
     aRatioKeepEx->setShortcut(QKeySequence("CTRL+4"));
     aRatioKeepEx->setProperty("mode", (int)Qt::KeepAspectRatioByExpanding);
     aRatioKeepEx->setCheckable(true);
     aRatioKeepEx->setActionGroup(m_gBackRatioActions);
     mScaling->addAction(aRatioKeepEx);
    QAction * aRatioKeep = new QAction(tr("Keep proportions"), mScaling);
     aRatioKeep->setShortcut(QKeySequence("CTRL+5"));
     aRatioKeep->setProperty("mode", (int)Qt::KeepAspectRatio);
     aRatioKeep->setCheckable(true);
     aRatioKeep->setActionGroup(m_gBackRatioActions);
     mScaling->addAction(aRatioKeep);
    QAction * aRatioIgnore = new QAction(tr("Ignore proportions"), mScaling);
     aRatioIgnore->setShortcut(QKeySequence("CTRL+6"));
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
    ui.exportButton->setProperty("printing", enabled);
}

void CanvasAppliance::slotAddCanvas()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);

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
    // disable any search box
    ui.aSearchPictures->setChecked(false);

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
    // disable any search box
    ui.aSearchPictures->setChecked(false);
    m_extCanvas->addTextContent();
}

void CanvasAppliance::slotAddWebcam()
{
    // disable any search box
    ui.aSearchPictures->setChecked(false);
    m_extCanvas->addWebcamContent(0);
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
    if (m_extCanvas->modeInfo()->projectMode() == CanvasModeInfo::ModeNormal)
        return ExportWizard(m_extCanvas).exec();
    return m_extCanvas->printAsImage(m_extCanvas->modeInfo()->printDpi(), m_extCanvas->modeInfo()->fixedPrinterPixels(), m_extCanvas->modeInfo()->printLandscape());
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
