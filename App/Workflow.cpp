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
#include "Shared/BreadCrumbBar.h"
#include "Shared/ButtonsDialog.h"
#include "App.h"
#include "Canvas/Canvas.h"
#include "CanvasAppliance.h"
#include "FotowallFile.h"
#include "HomeAppliance.h"
#include "Settings.h"
#include "WordcloudAppliance.h"

#include <QFileDialog>
#include <QTimer>


Workflow::Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent)
  : QObject(parent)
  , m_container(container)
  , m_bar(bar)
  , m_commandTimer(0)
  , m_processingQueue(false)
{
    // set the global reference
    App::workflow = this;

    // wire up the stacker and the bar
    connect(m_bar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotNodeClicked(quint32)));

    // load a fotowall file if asked from the command line
    QStringList contentUrls = App::settings->commandlineUrls();
    if (contentUrls.size() == 1 && App::isFotowallFile(contentUrls.first())) {
        if (loadCanvas(contentUrls.first()))
            return;
    }

    // load content in a canvas
    if (!contentUrls.isEmpty()) {
        Canvas * canvas = new Canvas(m_container->sceneViewSize(), this);
        canvas->addAutoContent(contentUrls);
        pushNode(new CanvasAppliance(canvas, m_container->physicalDpiX(), m_container->physicalDpiY()));
        return;
    }

    // show the home screen (if no valid commandline args)
    pushNode(new HomeAppliance);
}

Workflow::~Workflow()
{
    // unset the global reference
    App::workflow = 0;

    if (!m_stack.isEmpty()) {
        qWarning("Workflow::~Workflow: not empty!");
        clearNodes();
    }
    m_container = 0;

    // this is an example of 'autosave-like function'
    //QString tempPath = QDir::tempPath() + QDir::separator() + "autosave.fotowall";
    //m_canvas->saveToFile(tempPath);

    // bar and container are external: don't delete
}

bool Workflow::loadCanvas(const QString & __fileName)
{
    // ask for file name, if not provided - can CANCEL
    QString fileName = __fileName;
    if (fileName.isEmpty()) {
        QString defaultLoadPath = App::settings->value("Fotowall/LoadProjectDir").toString();
        fileName = QFileDialog::getOpenFileName(0, tr("Select the Fotowall file"), defaultLoadPath, tr("Fotowall (*.fotowall)"));
        if (fileName.isNull())
            return false;
        App::settings->setValue("Fotowall/LoadProjectDir", QFileInfo(fileName).absolutePath());
    }

    // schedule canvas loading
    scheduleCommand(Command::ResetToLevel);
    scheduleCommand(Command(Command::MasterCanvas, fileName));
    return true;
}

void Workflow::startCanvas()
{
    // schedule canvas creation
    scheduleCommand(Command::ResetToLevel);
    scheduleCommand(Command::MasterCanvas);
}\

void Workflow::startWordcloud()
{
    // schedule wordcloud creation
    scheduleCommand(Command::ResetToLevel);
    scheduleCommand(Command::MasterWordcloud);
}

void Workflow::startWizard()
{
    // TODO
    HERE
}

void Workflow::stackCanvasAppliance(const Resource & resource)
{
    // schedule slave canvas
    Command csc(Command::SlaveCanvas);
    csc.res.append(resource);
    scheduleCommand(csc);
}

void Workflow::stackWordcloudAppliance(const Resource &)
{
    // schedule slave canvas
//    Command csc(Command::SlaveWordcloud);
//    csc.res.append(resource);
//    scheduleCommand(csc);
    HERE
}

// OK
bool Workflow::applianceCommand(int command)
{
    if (!m_stack.isEmpty())
        m_stack.last().appliance->applianceCommand(command);
    return false;
}

bool Workflow::requestExit()
{
    // exit if no structure or on home screen
    if (m_stack.isEmpty() || dynamic_cast<HomeAppliance *>(m_stack.first().appliance))
        return true;

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


void Workflow::scheduleCommand(const Command &command)
{
    if (!m_commandTimer) {
        m_commandTimer = new QTimer(this);
        m_commandTimer->setInterval(0);
        m_commandTimer->setSingleShot(true);
        connect(m_commandTimer, SIGNAL(timeout()), this, SLOT(slotProcessQueue()));
    }
    m_commands.append(command);
    m_commandTimer->start();
}

bool Workflow::processCommand(const Workflow::Command & command)
{
    switch (command.type) {
        case Command::ResetToLevel: {
            int level = qMax(1, command.param.toInt());
            while (!dynamic_cast<HomeAppliance *>(m_stack.last().appliance) && m_stack.size() > level)
                popNode();
            }return true;

        case Command::MasterCanvas: {
            Canvas * canvas = new Canvas(m_container->sceneViewSize(), this);

            // load a file, if in params
            if (command.param.type() == QVariant::String) {
                QString fileName = command.param.toString();
                if (!FotowallFile::read(fileName, canvas, true))
                    qWarning("Workflow::processCommand: MasterCanvas: can't load canvas. Display error?");
            }

            // create the canvas appliance
            CanvasAppliance * canvasApp = new CanvasAppliance(canvas, m_container->physicalDpiX(), m_container->physicalDpiY());
            pushNode(canvasApp);
            } return true;

        case Command::MasterWordcloud: {
            Wordcloud::Cloud * cloud = new Wordcloud::Cloud(this);

            // TODO: load from file
            if (command.param.type() == QVariant::String)
                HERE;

            // create the wordcloud appliance
            WordcloudAppliance * wcApp = new WordcloudAppliance(cloud, this);
            pushNode(wcApp);
            } return true;

        case Command::SlaveCanvas: {
            // get the canvas out of the first resource
            const Resource & resource = command.res.first();
            Canvas * canvas = static_cast<CanvasAppliance *>(resource.first)->borrowCanvas(resource.second);

            // create the
            CanvasAppliance * canvasApp = new CanvasAppliance(canvas, m_container->physicalDpiX(), m_container->physicalDpiY());
            Node node(canvasApp);
            node.res = command.res;
            pushNode(node);
            } return true;

    }

    // catch errors
    return false;
}


// OK
void Workflow::pushNode(const Node & node)
{
    // remove previous Appliance from container
    if (!m_stack.isEmpty() && m_container)
        m_stack.last().appliance->removeFromApplianceContainer();
    m_stack.append(node);
    // show this Appliance on container
    if (m_container)
        node.appliance->addToApplianceContainer(m_container);
    updateBreadcrumb();
}

void Workflow::popNode()
{
    // delete last
    if (!m_stack.isEmpty()) {
        const Node node = m_stack.takeLast();
        PlugGui::AbstractAppliance * app = node.appliance;

        // restore external references
        ResourceList::const_iterator rIt = node.res.begin();
        for (; rIt != node.res.end(); ++rIt) {
            const Resource & res = *rIt;
            PlugGui::AbstractAppliance * reqAppliance = res.first;
            QVariant reqKey = res.second;
            if (CanvasAppliance * cApp = dynamic_cast<CanvasAppliance *>(reqAppliance))
                cApp->returnCanvas(reqKey, static_cast<CanvasAppliance *>(app)->takeCanvas());
            else
                qWarning("Workflow::popNode: releasing to appliance '%s' not handled", qPrintable(reqAppliance->applianceName()));
        }

        // TODO - SAVE/SERIALIZE/OTHER LINK HERE

        if (m_container)
            app->removeFromApplianceContainer();
        delete app;
    }

    // show the last-1
    if (m_container && !m_stack.isEmpty())
        m_stack.last().appliance->addToApplianceContainer(m_container);

    updateBreadcrumb();
}

void Workflow::clearNodes()
{
    while (!m_stack.isEmpty())
        popNode();
}

void Workflow::updateBreadcrumb()
{
    // build the new breadcrumbbar's contents
    m_bar->clearNodes();
    if (m_stack.size() > 1) {
        quint32 index = 0;
        foreach (const Node & node, m_stack) {
            m_bar->addNode(index + 1, node.appliance->applianceName(), index);
            index++;
        }
    }

    // issue a complete repaint
    m_container->update();
}


void Workflow::slotNodeClicked(quint32 level)
{
    scheduleCommand(Command(Command::ResetToLevel, level));
}

void Workflow::slotProcessQueue()
{
    if (m_processingQueue) {
        qWarning("Workflow::slotProcessQueue: nesting detected -FIXME-");
        return;
    }
    m_processingQueue = true;
    while (!m_commands.isEmpty())
        processCommand(m_commands.takeFirst());
    m_processingQueue = false;
}
