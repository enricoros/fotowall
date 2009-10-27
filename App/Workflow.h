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
#include "Shared/PlugGui/Stacker.h"
#include <QVariant>
namespace Wordcloud { class Cloud; }
class BreadCrumbBar;
class Canvas;

class WorkflowRequest {
    public:
        // type of request and parameter
        enum Type {
            NewCanvas,          // param is empty or filename or canvas star
            StackCanvas,        // param is canvas star
            StackWordcloud      // param is wordcloud star
        }               type;
        QVariant        param;

        // notification
        QObject *       listener;
        const char *    listenerEntrySlot;
        const char *    listenerExitSlot;

        // convenience initializer
        WorkflowRequest(Type type, const QVariant & param = QVariant(),
                        QObject * listener = 0, const char * listenerEntry = 0,
                        const char * listenerExit = 0);
    private:
        WorkflowRequest();
};

class Workflow : public QObject, public PlugGui::Stacker
{
    Q_OBJECT
    public:
        Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent = 0);
        ~Workflow();

        // some requests will be performed asynchronously
        void request(const WorkflowRequest & request);

        // ### BIG REFACTOR HERE ;-)
        bool requestExit();
        bool applianceCommand(int command);

        // ### BIG REFACTOR HERE ;-)
        bool loadCanvas(const QString & fileName);
        void startCanvas();
        void startWordcloud();
        void startWizard();
        void stackCanvasAppliance(Canvas * newCanvas);
        void stackWordcloudAppliance(Wordcloud::Cloud * cloud);

    protected:
        // ::PlugGui::Stacker
        void structureChanged();

    private:
        void showHome();

/*        Container * m_container;
        QList<AbstractAppliance *> m_appliances;

        struct Token {
            AbstractAppliance * appliance;
            QObject * listener;
        };*/

        // external objects
        PlugGui::Container * m_container;
        BreadCrumbBar * m_bar;

    private Q_SLOTS:
        void slotApplianceClicked(quint32);
        //void slotNextStep
};

#endif
