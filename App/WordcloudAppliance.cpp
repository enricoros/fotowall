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

#include "WordcloudAppliance.h"

#include "Shared/AbstractScene.h"

WordcloudAppliance::WordcloudAppliance(WordCloud::Cloud * cloud, QObject * parent)
  : Appliance::AbstractAppliance(parent)
  , m_cloud(cloud)
  , m_scene(new AbstractScene(this))
  , ui(0)
  , m_dummyWidget(0)
{
    // init UI
    ui = new Ui::WordcloudApplianceElements;
    m_dummyWidget = new QWidget;
    ui->setupUi(m_dummyWidget);

    // set the target scene of the cloud to this
    cloud->setScene(m_scene);

    // configure the appliance
    sceneSet(m_scene);
}

WordCloud::Cloud * WordcloudAppliance::cloud() const
{
    return m_cloud;
}
