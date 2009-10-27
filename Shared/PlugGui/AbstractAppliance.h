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

#ifndef __AbstractAppliance_h__
#define __AbstractAppliance_h__

#include <QObject>
#include <QPointer>
#include <QVariant>
#include "../AbstractScene.h"
#include "Container.h"

namespace PlugGui {

    /**
        \brief A collection of widgets that can be plugged into a Container
    */
    class AbstractAppliance : public QObject
    {
        public:
            AbstractAppliance(QObject * parent = 0);

            // plugging into a container
            bool addToApplianceContainer(Container * container);
            void removeFromApplianceContainer();

            // appliance description and external control
            virtual QString applianceName() const = 0;
            virtual bool applianceCommand(int command);

        protected:
            // used by reimpls to access the container
            void sceneSet(AbstractScene *);
            void sceneClear();
            void topbarAddWidget(QWidget *, bool rightBar = false, int index = -1);
            void topbarRemoveWidget(QWidget *);
            void sidebarSetWidget(QWidget *);
            void sidebarClearWidget();
            void centralwidgetSet(QWidget *);
            void centralwidgetClear();
            void containerValueSet(quint32 key, const QVariant & value);

        private:
            void updateContainerTopbar();
            void detachFromContainer();

            typedef QPointer<QWidget> WidgetPointer;
            typedef QPointer<AbstractScene> ScenePointer;
            typedef QPointer<Container> ContainerPointer;
            typedef QMap<int, QVariant> ValueMap;

            ScenePointer m_pScene;
            QList<WidgetPointer> m_pTopbar;
            WidgetPointer m_pSidebar;
            WidgetPointer m_pCentral;
            ContainerPointer m_containerPtr;
            ValueMap m_values;
    };

}

#endif
