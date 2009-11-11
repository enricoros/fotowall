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
        if (loadCanvas_A(contentUrls.first()))
            return;
    }

    // load content in a canvas
    if (!contentUrls.isEmpty()) {
        Canvas * canvas = new Canvas(m_container->physicalDpiX(), m_container->physicalDpiY(), this);
        canvas->resize(m_container->sceneViewSize());
        canvas->addAutoContent(contentUrls);
        pushNode(new CanvasAppliance(canvas));
        return;
    }

    // show the home screen (if no valid commandline args)
    pushNode(new HomeAppliance);
}

Workflow::~Workflow()
{
    // warn if we have any appliance left, shouldn't be
    while (!m_stack.isEmpty()) {
        qWarning("Workflow::~Workflow: not empty stack. dropping all");
        popNode(true);
    }

    // unset the global reference
    App::workflow = 0;
    m_container = 0;

    // this is an example of 'autosave-like function'
    //QString tempPath = QDir::tempPath() + QDir::separator() + "autosave.fotowall";
    //m_canvas->saveToFile(tempPath);

    // bar and container are external: don't delete
}

bool Workflow::loadCanvas_A(const QString & givenName)
{
    // ask for file name, if not provided - can CANCEL
    QString fileName = givenName.isEmpty() ? FotowallFile::getLoadFotowallFile() : givenName;
    if (fileName.isEmpty())
        return false;

    // schedule canvas loading
    scheduleCommand(Command::ResetToLevel);
    scheduleCommand(Command(Command::MasterCanvas, fileName));
    return true;
}

void Workflow::startCanvas_A()
{
    // schedule canvas creation
    scheduleCommand(Command::ResetToLevel);
    scheduleCommand(Command::MasterCanvas);
}

void Workflow::startWordcloud_A()
{
    // schedule wordcloud creation
    scheduleCommand(Command::ResetToLevel);
    scheduleCommand(Command::MasterWordcloud);
}

void Workflow::stackSlaveCanvas_A(SingleResourceLoaner * resource)
{
    // schedule slave canvas
    scheduleCommand(Command(Command::SlaveCanvas, QVariant(), resource));
}

void Workflow::stackSlaveWordcloud_A(SingleResourceLoaner * resource)
{
    // schedule slave wordcloud
    scheduleCommand(Command(Command::SlaveWordcloud, QVariant(), resource));
}

bool Workflow::applianceCommand(int command)
{
    if (!m_stack.isEmpty())
        m_stack.last().appliance->applianceCommand(command);
    return false;
}

bool Workflow::requestExit()
{
    // count master appliances that can be saved
    int requiringSave = 0;
    foreach (const Node & node, m_stack) {
        // skip slaves
        if (node.res)
            continue;

        // count modified appliances
        if (CanvasAppliance * cApp = dynamic_cast<CanvasAppliance *>(node.appliance)) {
            if (cApp->pendingChanges())
                ++requiringSave;
        } else if (WordcloudAppliance * wApp = dynamic_cast<WordcloudAppliance *>(node.appliance)) {
            if (wApp->pendingChanges())
                ++requiringSave;
        }
    }

    // build the closure dialog
    ButtonsDialog quitAsk("Workflow-Exit", tr("Closing Fotowall..."));
    quitAsk.setMinimumWidth(350);
    quitAsk.setButtonText(QDialogButtonBox::Cancel, tr("Cancel"));
    if (requiringSave) {
        quitAsk.setMessage(tr("Are you sure you want to quit and lose your changes?"));
        quitAsk.setButtonText(QDialogButtonBox::Save, tr("Save"));
        quitAsk.setButtonText(QDialogButtonBox::Close, tr("Don't Save"));
        quitAsk.setButtons(QDialogButtonBox::Save | QDialogButtonBox::Close | QDialogButtonBox::Cancel);
    } else {
        quitAsk.setMessage(tr("Are you sure you want to quit?"));
        quitAsk.setButtonText(QDialogButtonBox::Close, tr("Quit"));
        quitAsk.setButtons(QDialogButtonBox::Close | QDialogButtonBox::Cancel);
    }

    // react to the dialog's answer
    QDialogButtonBox::StandardButton button = quitAsk.execute();

    // handle Cancel
    if (button == QDialogButtonBox::Cancel)
        return false;

    // handle Save
    if (button == QDialogButtonBox::Save) {
        while (!m_stack.isEmpty())
            popNode(false);
        return true;
    }

    // handle Quit without saving
    while (!m_stack.isEmpty())
        popNode(true);
    return true;
}

void Workflow::scheduleCommand(const Command &command)
{
    if (!m_commandTimer) {
        m_commandTimer = new QTimer(this);
        m_commandTimer->setSingleShot(true);
        connect(m_commandTimer, SIGNAL(timeout()), this, SLOT(slotProcessQueue()));
    }
    m_commands.append(command);
    m_commandTimer->start(0);
}

bool Workflow::processCommand(const Workflow::Command & command)
{
    switch (command.type) {
        case Command::ResetToLevel: {
            int level = qMax(1, command.param.toInt());
            while (!dynamic_cast<HomeAppliance *>(m_stack.last().appliance) && m_stack.size() > level)
                popNode(false);
            }return true;

        case Command::MasterCanvas: {
            Canvas * canvas = new Canvas(m_container->physicalDpiX(), m_container->physicalDpiY(), this);

            // load a file, if in params
            if (command.param.type() == QVariant::String) {
                QString fileName = command.param.toString();
                if (!FotowallFile::read(fileName, canvas, true))
                    qWarning("Workflow::processCommand: MasterCanvas: can't load canvas. Display error?");
            }

            // create the canvas appliance
            CanvasAppliance * canvasApp = new CanvasAppliance(canvas);
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
            // get the Canvas out of the resource
            Canvas * canvas = static_cast<Canvas *>(qVariantValue<void *>(command.res->takeResource()));

            // create the canvas appliance
            if (canvas) {
                CanvasAppliance * canvasApp = new CanvasAppliance(canvas);
                pushNode(Node(canvasApp, command.res));
            }
            } return true;

        case Command::SlaveWordcloud: {
            // get the Cloud out of the resource
            Wordcloud::Cloud * cloud = static_cast<Wordcloud::Cloud *>(qVariantValue<void *>(command.res->takeResource()));

            // create the wordcloud appliance
            WordcloudAppliance * wcApp = new WordcloudAppliance(cloud, this);
            pushNode(Node(wcApp, command.res));
            } return true;
    }

    // catch errors
    return false;
}

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

void Workflow::popNode(bool discardChanges)
{
    // delete last
    if (!m_stack.isEmpty()) {
        const Node node = m_stack.takeLast();
        PlugGui::AbstractAppliance * app = node.appliance;

        // master: save content
        if (!node.res) {
            if (CanvasAppliance * cApp = dynamic_cast<CanvasAppliance *>(app)) {
                if (!discardChanges && cApp->pendingChanges())
                    cApp->saveToFile();
                delete cApp->takeCanvas();
            } else if (WordcloudAppliance * wApp = dynamic_cast<WordcloudAppliance *>(app)) {
                if (!discardChanges)
                    wApp->saveToFile();
                delete wApp->takeCloud();
            } else if (dynamic_cast<HomeAppliance *>(app)) {
                // no data to delete here
            } else
                qWarning("Workflow::popNode: saving of appliance '%s' not handled", qPrintable(app->applianceName()));
        }

        // slaves: store external references
        else {
            if (CanvasAppliance * cApp = dynamic_cast<CanvasAppliance *>(app)) {
                Canvas * modifiedCanvas = cApp->takeCanvas();
                node.res->returnResource(qVariantFromValue((void *)modifiedCanvas));
            } else if (WordcloudAppliance * wApp = dynamic_cast<WordcloudAppliance *>(app)) {
                Wordcloud::Cloud * modifiedCloud = wApp->takeCloud();
                node.res->returnResource(qVariantFromValue((void *)modifiedCloud));
            } else
                qWarning("Workflow::popNode: releasing of appliance '%s' not handled", qPrintable(app->applianceName()));
        }

        if (m_container)
            app->removeFromApplianceContainer();
        delete app;
    }

    // show the last-1
    if (m_container && !m_stack.isEmpty())
        m_stack.last().appliance->addToApplianceContainer(m_container);

    updateBreadcrumb();
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
