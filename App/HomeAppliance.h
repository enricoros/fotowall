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

#ifndef __HomeAppliance_h__
#define __HomeAppliance_h__

#include "Shared/PlugGui/AbstractAppliance.h"
#include "Shared/AbstractScene.h"
class HomeScene;
class UrlHistoryBox;

class HomeAppliance : public PlugGui::AbstractAppliance
{
    Q_OBJECT
    public:
        HomeAppliance(QObject * parent = 0);
        ~HomeAppliance();

        // ::Appliance::AbstractAppliance
        QString applianceName() const { return tr("Home"); }

    private:
        HomeScene * m_scene;
        UrlHistoryBox * m_historyBox;

    private Q_SLOTS:
        void slotLoadUrl(const QUrl & url);
};


class HomeScene : public AbstractScene
{
    Q_OBJECT
    public:
        HomeScene(QObject * parent = 0);
        ~HomeScene();

        // ::QGraphicsScene
        void drawBackground(QPainter *painter, const QRectF &rect);

        // ::AbstractScene
        void resize(const QSize & size);

    private:
        QList<QGraphicsItem *> m_labels;

    private Q_SLOTS:
        void slotNewCanvas();
        void slotNewWordcloud();
        void slotWizard();
};

#endif
