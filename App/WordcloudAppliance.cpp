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

#include <QPushButton>

WordcloudAppliance::WordcloudAppliance(WordCloud::Cloud * cloud, QObject * parent)
  : Appliance::AbstractAppliance(parent)
  , m_cloud(cloud)
  , m_scene(new AbstractScene)
  , ui(new Ui::WordcloudApplianceElements)
  , m_dummyWidget(new QWidget)
{
    // init UI
    ui->setupUi(m_dummyWidget);

    // set the target scene of the cloud to this
    cloud->setScene(m_scene);

    QPushButton * btn = new QPushButton(tr("Randomize"));
    connect(btn, SIGNAL(clicked()), this, SLOT(slotRandomizeCloud()));

    // configure the appliance
    sceneSet(m_scene);
    sidebarSetWidget(btn);
}

WordcloudAppliance::~WordcloudAppliance()
{
    qWarning("WCA closed");
    delete m_scene;
    delete ui;
    delete m_dummyWidget;
}

WordCloud::Cloud * WordcloudAppliance::cloud() const
{
    return m_cloud;
}

void WordcloudAppliance::slotRegenCloud()
{
    m_cloud->regenCloud();
}

void WordcloudAppliance::slotRandomizeCloud()
{
    m_cloud->randomCloud();
}
