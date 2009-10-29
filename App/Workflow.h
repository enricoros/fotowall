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

#ifndef __Workflow_h__
#define __Workflow_h__

#include <QObject>
#include <QVariant>
#include "Shared/PlugGui/AbstractAppliance.h"
namespace Wordcloud { class Cloud; }
class BreadCrumbBar;
class Canvas;

// TODO: add checkes to avoid enqueueing while processing a command, or a list

class Workflow : public QObject
{
    Q_OBJECT
    public:
        Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent = 0);
        ~Workflow();

        typedef QPair<PlugGui::AbstractAppliance *, QVariant> Resource;
        typedef QList<Resource> ResourceList;

        // change workflow
        bool loadCanvas(const QString & fileName = QString());
        void startCanvas();
        void startWordcloud();
        void startWizard();
        void stackCanvasAppliance(const Resource & resource);
        void stackWordcloudAppliance(const Resource & resource);

        bool applianceCommand(int command);
        bool requestExit();

    private:
        // TEMP
        struct Command {
            enum Type { ResetToLevel, MasterCanvas, MasterWordcloud, SlaveCanvas };

            Type type;
            QVariant param;
            ResourceList res;

            Command(Type type, const QVariant & param = QVariant()) : type(type), param(param) {}
        };
        void scheduleCommand(const Command & command);
        bool processCommand(const Command & command);

        struct Node {
            PlugGui::AbstractAppliance * appliance;
            ResourceList res;

            Node(PlugGui::AbstractAppliance * appliance) : appliance(appliance) {}
        };

        void pushNode(const Node & node);
        void popNode();
        void clearNodes();
        void updateBreadcrumb();

        // external objects
        PlugGui::Container * m_container;
        BreadCrumbBar * m_bar;

        // commands
        QTimer * m_commandTimer;
        QList<Command> m_commands;
        bool m_processingQueue;

        // nodes structure
        QList<Node> m_stack;

    private Q_SLOTS:
        void slotNodeClicked(quint32);
        void slotProcessQueue();
};

#endif
