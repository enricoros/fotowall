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
#include "Shared/VideoProvider.h"
#include "App.h"
#include "ExactSizeDialog.h"
#include "SceneView.h"
#include "Settings.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>


CanvasAppliance::CanvasAppliance(Canvas * canvas, SceneView * view, QObject * parent)
  : Appliance::AbstractAppliance(parent)
  , m_canvas(canvas)
  , ui(0)
  , m_dummyWidget(0)
  , m_gBackActions(0)
  , m_gBackRatioActions(0)
{
    // init UI
    ui = new Ui::CanvasApplianceElements;
    m_dummyWidget = new QWidget;
    ui->setupUi(m_dummyWidget);
    ui->b1->setDefaultAction(ui->aAddPicture);
    ui->b2->setDefaultAction(ui->aAddText);
    ui->b3->setDefaultAction(ui->aAddWebcam);
    ui->b4->setDefaultAction(ui->aAddFlickr);
    ui->b5->setDefaultAction(ui->aAddCanvas);
    ui->b6->setDefaultAction(ui->aAddWordCloud);
    connect(ui->aAddPicture, SIGNAL(triggered()), this, SLOT(slotAddPicture()));
    connect(ui->aAddText, SIGNAL(triggered()), this, SLOT(slotAddText()));
    connect(ui->aAddWebcam, SIGNAL(triggered()), this, SLOT(slotAddWebcam()));
    connect(ui->aAddFlickr, SIGNAL(toggled(bool)), this, SLOT(slotAddFlickrToggled(bool)));
    connect(ui->aAddCanvas, SIGNAL(triggered()), this, SLOT(slotAddCanvas()));
    connect(ui->aAddWordCloud, SIGNAL(triggered()), this, SLOT(slotAddWordCloud()));
    ui->propertiesBox->collapse();
    ui->canvasPropertiesBox->expand();
    connect(ui->projectType, SIGNAL(activated(int)), this, SLOT(slotProjectTypeActivated(int)));

    // configure the appliance
    sceneSet(m_canvas);
    topbarAddWidget(ui->addContentBox);
    topbarAddWidget(ui->propertiesBox);
    topbarAddWidget(ui->canvasPropertiesBox);

    // populate menus
    ui->arrangeButton->setMenu(createArrangeMenu());
    ui->backButton->setMenu(createBackgroundMenu());
    ui->decoButton->setMenu(createDecorationMenu());

    // react to canvas
    m_canvas->modeInfo()->setScreenDpi(view->physicalDpiX(), view->physicalDpiY());
    m_canvas->modeInfo()->setPrintDpi(300);
    connect(m_canvas, SIGNAL(refreshCanvas()), view, SLOT(sceneConstraintsUpdated()));
    connect(m_canvas, SIGNAL(refreshCanvas()), this, SLOT(slotRefreshCanvas()));
    connect(m_canvas, SIGNAL(backModeChanged()), this, SLOT(slotBackModeChanged()));
    connect(m_canvas, SIGNAL(showPropertiesWidget(QWidget*)), this, SLOT(slotShowPropertiesWidget(QWidget*)));

    // react to VideoProvider
    ui->aAddWebcam->setVisible(VideoProvider::instance()->inputCount() > 0);
    connect(VideoProvider::instance(), SIGNAL(inputCountChanged(int)), this, SLOT(slotVerifyVideoInputs(int)));

    // set the startup project mode
    slotProjectTypeActivated(0);
}

Canvas * CanvasAppliance::canvas() const
{
    return m_canvas;
}

QMenu * CanvasAppliance::createArrangeMenu()
{
    QMenu * menu = new QMenu();

    QAction * aForceField = new QAction(tr("Enable force field"), menu);
    aForceField->setCheckable(true);
    aForceField->setChecked(m_canvas->forceFieldEnabled());
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

QMenu * CanvasAppliance::createDecorationMenu()
{
    QMenu * menu = new QMenu();

    QAction * aTop = new QAction(tr("Top bar"), menu);
    aTop->setCheckable(true);
    if (m_canvas) // FIXME: bind to a property
        aTop->setChecked(m_canvas->topBarEnabled());
    connect(aTop, SIGNAL(toggled(bool)), this, SLOT(slotDecoTopBar(bool)));
    menu->addAction(aTop);

    QAction * aBottom = new QAction(tr("Bottom bar"), menu);
    aBottom->setCheckable(true);
    if (m_canvas) // FIXME: bind to a property
        aBottom->setChecked(m_canvas->bottomBarEnabled());
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
    m_canvas->modeInfo()->setFixedSizeInches();
    m_canvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeNormal);
    /// FIXME ui->exportButton->setText(tr("Export"));
    ui->projectType->setCurrentIndex(0);
}

void CanvasAppliance::setCDProject()
{
    m_canvas->modeInfo()->setFixedSizeInches(QSizeF(4.75, 4.75));
    m_canvas->modeInfo()->setPrintLandscape(false);
    m_canvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeCD);
    /// FIXME ui->exportButton->setText(tr("Print"));
    ui->projectType->setCurrentIndex(1);
}

void CanvasAppliance::setDVDProject()
{
    m_canvas->modeInfo()->setFixedSizeInches(QSizeF(10.83, 7.2));
    m_canvas->modeInfo()->setPrintLandscape(true);
    m_canvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeDVD);
    /// FIXME ui->exportButton->setText(tr("Print"));
    ui->projectType->setCurrentIndex(2);
}

void CanvasAppliance::setExactSizeProject()
{
    if (!m_canvas->modeInfo()->fixedSize()) {
        ExactSizeDialog sizeDialog;
        QPointF screenDpi = m_canvas->modeInfo()->screenDpi();
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
        m_canvas->modeInfo()->setPrintLandscape(landscape);
        m_canvas->modeInfo()->setPrintDpi(printDpi);
        m_canvas->modeInfo()->setFixedSizeInches(QSizeF(cm?(double)w/2.54:w, cm?(double)h/2.54:h));
    }
    m_canvas->modeInfo()->setProjectMode(CanvasModeInfo::ModeExactSize);
    /// FIXME ui->exportButton->setText(tr("Print"));
    ui->projectType->setCurrentIndex(3);
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
    m_canvas->addCanvasViewContent(fileNames);
}

void CanvasAppliance::slotAddFlickrToggled(bool on)
{
    m_canvas->setWebContentSelectorVisible(on);
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
    m_canvas->addPictureContent(fileNames);
}

void CanvasAppliance::slotAddText()
{
    m_canvas->addTextContent();
}

void CanvasAppliance::slotAddWebcam()
{
    m_canvas->addWebcamContent(0);
}

void CanvasAppliance::slotAddWordCloud()
{
    m_canvas->addWordCloudContent();
}


void CanvasAppliance::slotProjectTypeActivated(int index)
{
    switch (index) {
        case 0: setNormalProject();     break;
        case 1: setCDProject();         break;
        case 2: setDVDProject();        break;
        case 3: m_canvas->modeInfo()->setFixedSizeInches();
                setExactSizeProject();  break;
    }
    // HACK
    //QResizeEvent ev(ui->sceneView->size(), ui->sceneView->size());
    //ui->sceneView->resizeEvent(&ev);
}

void CanvasAppliance::slotSetBackMode(QAction* action)
{
    int choice = action->property("id").toUInt();
    m_canvas->setBackMode(choice);
}

void CanvasAppliance::slotSetBackRatio(QAction* action)
{
    Qt::AspectRatioMode mode = (Qt::AspectRatioMode)action->property("mode").toInt();
    m_canvas->setBackContentRatio(mode);
}

void CanvasAppliance::slotArrangeForceField(bool checked)
{
    m_canvas->setForceFieldEnabled(checked);
}

#include "Canvas/AbstractContent.h"
void CanvasAppliance::slotArrangeRandom()
{
    QRectF r = m_canvas->sceneRect();
    foreach (QGraphicsItem * item, m_canvas->items()) {
        AbstractContent * content = dynamic_cast<AbstractContent *>(item);
        if (!content)
            continue;
        content->setPos(r.left() + (qrand() % (int)r.width()), r.top() + (qrand() % (int)r.height()));
        content->setRotation(-30 + (qrand() % 60), Qt::ZAxis);
#if QT_VERSION >= 0x040500
        content->setOpacity((qreal)(qrand() % 100) / 99.0);
#endif
    }
}

void CanvasAppliance::slotDecoTopBar(bool checked)
{
    m_canvas->setTopBarEnabled(checked);
}

void CanvasAppliance::slotDecoBottomBar(bool checked)
{
    m_canvas->setBottomBarEnabled(checked);
}

void CanvasAppliance::slotDecoSetTitle()
{
    // set a dummy title, if none
    if (m_canvas->titleText().isEmpty())
        m_canvas->setTitleText("...");

    // change title dialog
    bool ok = false;
    QString title = QInputDialog::getText(0, tr("Title"), tr("Insert the title"), QLineEdit::Normal, m_canvas->titleText(), &ok);
    if (ok)
        m_canvas->setTitleText(title);
}

void CanvasAppliance::slotDecoClearTitle()
{
    m_canvas->setTitleText(QString());
}

void CanvasAppliance::slotRefreshCanvas()
{
    int mode = m_canvas->modeInfo()->projectMode();
    if (mode == 3) { // If exact size project
        // Called here not to have the unneeded size dialog
#warning FIFFFFFF
        // FIXME setExactSizeProject();
    } else {
        slotProjectTypeActivated(mode);
    }
}

void CanvasAppliance::slotBackModeChanged()
{
    int mode = m_canvas->backMode();
    m_gBackActions->actions()[mode - 1]->setChecked(true);
    m_gBackActions->actions()[2]->setEnabled(mode == 3);
}

void CanvasAppliance::slotBackRatioChanged()
{
    Qt::AspectRatioMode mode = m_canvas->backContentRatio();
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
    QLayoutItem * prevItem = ui->propLayout->takeAt(0);
    if (prevItem) {
        delete prevItem->widget();
        delete prevItem;
    }

    // show the Properties container with new content and title
    if (widget) {
        ui->canvasPropertiesBox->collapse();
        widget->setParent(ui->propertiesBox);
        ui->propLayout->addWidget(widget);
        ui->propertiesBox->setTitle(widget->windowTitle());
        ui->propertiesBox->expand();
    }
    // or show the Canvas container
    else {
        ui->propertiesBox->collapse();
        ui->canvasPropertiesBox->expand();
    }
}

void CanvasAppliance::slotVerifyVideoInputs(int count)
{
    // maybe blink or something?
    ui->aAddWebcam->setVisible(count > 0);
}
