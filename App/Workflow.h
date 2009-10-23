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
namespace PlugGui { class Container; class Stacker; }
namespace Wordcloud { class Cloud; }
class BreadCrumbBar;
class Canvas;

class Workflow : public QObject
{
    Q_OBJECT
    public:
        Workflow(PlugGui::Container * container, BreadCrumbBar * bar, QObject * parent = 0);
        ~Workflow();

        // ### BIG REFACTOR HERE ;-)
        void clear();
        bool saveCurrent();
        bool exportCurrent();
        void howtoCurrent();
        void clearBackgroundCurrent();

        // content editing
        void newCanvas();
        bool loadCanvas(const QString & fileName);
        void stackCanvasAppliance(Canvas * newCanvas);
        void stackWordcloudAppliance(Wordcloud::Cloud * cloud);

        bool requestExit();

    private:
        void workflowChanged();

        // external objects
        PlugGui::Container * m_container;
        BreadCrumbBar * m_bar;

        // internals
        PlugGui::Stacker * m_stacker;

    private Q_SLOTS:
        void slotStackChanged();
        void slotApplianceClicked(quint32);
};

#endif
