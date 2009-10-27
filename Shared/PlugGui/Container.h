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

#ifndef __ApplianceContainer_h__
#define __ApplianceContainer_h__

#include <QWidget>
#include <QSize>
class AbstractScene;

namespace PlugGui {

    class AbstractAppliance;

    class Container : public QWidget
    {
        Q_OBJECT
        public:
            Container(QWidget * parent = 0);

            // const queries to the container
            virtual QSize sceneViewSize() const = 0;

        protected:
            // called by appliances
            friend class PlugGui::AbstractAppliance;
            virtual void applianceSetScene(AbstractScene * scene) = 0;
            virtual void applianceSetTopbar(const QList<QWidget *> & widgets) = 0;
            virtual void applianceSetSidebar(QWidget * widget) = 0;
            virtual void applianceSetCentralwidget(QWidget * widget) = 0;
            virtual void applianceSetValue(quint32 key, const QVariant & value) = 0;
    };

}

#endif
