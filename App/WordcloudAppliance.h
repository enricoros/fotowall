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

#ifndef __WordcloudAppliance_h__
#define __WordcloudAppliance_h__

#include "Shared/AbstractAppliance.h"
#include "WordCloud/WordCloud.h"
#include "ui_WordcloudAppliance.h"
class AbstractScene;
class QMenu;

class WordcloudAppliance : public Appliance::AbstractAppliance
{
    Q_OBJECT
    public:
        WordcloudAppliance(WordCloud::Cloud * cloud, QObject * parent = 0);
        ~WordcloudAppliance();

        // peek into the cloud
        WordCloud::Cloud * cloud() const;

        // ::Appliance::AbstractAppliance
        QString applianceName() const { return tr("Word Cloud"); }
        int applianceElements() const { return Appliance::UseAllElements; }
        bool applianceCommand(int /*command*/) { return false; }

    private:
        WordCloud::Cloud * m_cloud;
        AbstractScene * m_scene;
        Ui::WordcloudApplianceElements * ui;
        QWidget * m_dummyWidget;

    private Q_SLOTS:
        void slotRegenCloud();
        void slotRandomizeCloud();
};

#endif
