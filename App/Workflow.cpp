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

#include "Workflow.h"

#include "Shared/PlugGui/Container.h"
#include "Shared/PlugGui/Stacker.h"
#include "Shared/BreadCrumbBar.h"
#include "Shared/ButtonsDialog.h"
#include "App.h"
#include "Canvas/Canvas.h"
#include "CanvasAppliance.h"
#include "FotowallFile.h"
#include "Settings.h"
#include "WordcloudAppliance.h"

Workflow::Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent)
  : QObject(parent)
  , m_container(container)
  , m_bar(bar)
  , m_stacker(new PlugGui::Stacker)
{
    // set the global reference
    App::workflow = this;

    // wire up the stacker and the bar
    m_stacker->setContainer(container);
    connect(m_stacker, SIGNAL(structureChanged()), this, SLOT(slotStackChanged()));
    connect(m_bar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotApplianceClicked(quint32)));

    // ###
    newCanvas();
}

Workflow::~Workflow()
{
    // unset the global reference
    App::workflow = 0;

    // this is an example of 'autosave-like function'
    //QString tempPath = QDir::tempPath() + QDir::separator() + "autosave.fotowall";
    //FotowallFile::saveV2(tempPath, m_canvas);

    // bar and container are external
    delete m_stacker;
}

void Workflow::clear()
{
    m_stacker->clearAppliances();
}

bool Workflow::saveCurrent()
{
    return m_stacker->currentApplianceCommand(App::AC_Save);
}

bool Workflow::exportCurrent()
{
    return m_stacker->currentApplianceCommand(App::AC_Export);
}

void Workflow::howtoCurrent()
{
    m_stacker->currentApplianceCommand(App::AC_ShowIntro);
}

void Workflow::clearBackgroundCurrent()
{
    m_stacker->currentApplianceCommand(App::AC_ClearBackground);
}

void Workflow::newCanvas()
{
    clear();

    QStringList contentUrls = App::settings->commandlineUrls();
    Canvas * canvas = new Canvas(m_container->sceneViewSize(), this);

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

    stackCanvasAppliance(canvas);
}

bool Workflow::loadCanvas(const QString & fileName)
{
    // load the canvas from file
    Canvas * canvas = new Canvas(m_container->sceneViewSize(), this);
    if (!FotowallFile::read(fileName, canvas)) {
        delete canvas;
        return false;
    }

    // close all and edit it
    clear();
    stackCanvasAppliance(canvas);
    return true;
}


void Workflow::stackCanvasAppliance(Canvas * canvas)
{
    CanvasAppliance * cApp = new CanvasAppliance(canvas, m_container->physicalDpiX(), m_container->physicalDpiY(), this);
    m_stacker->stackAppliance(cApp);
}

void Workflow::stackWordcloudAppliance(Wordcloud::Cloud * cloud)
{
    WordcloudAppliance * wApp = new WordcloudAppliance(cloud, this);
    m_stacker->stackAppliance(wApp);
}

bool Workflow::requestExit()
{
    // build the closure dialog
    ButtonsDialog quitAsk("Workflow-Exit", tr("Closing Fotowall..."));
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
            return false;
#if 0
        case QDialogButtonBox::Save:
            // save file and return to Fotowall if canceled
            if (!saveCurrent())
                return false;
            // fall through
#endif
        default:
            return true;
    }
}

void Workflow::slotStackChanged()
{
    // build the new breadcrumbbar's contents
    m_bar->clearNodes();
    QList<PlugGui::AbstractAppliance *> appliances = m_stacker->stackedAppliances();
    if (appliances.size() >= 2) {
        quint32 index = 0;
        foreach (PlugGui::AbstractAppliance * app, appliances) {
            m_bar->addNode(index + 1, app->applianceName(), index);
            index++;
        }
    }

    // issue a complete repaint
    m_container->update();
}

void Workflow::slotApplianceClicked(quint32 id)
{
    m_stacker->dropStackAfter(id - 1);
}
