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

WordcloudAppliance::WordcloudAppliance(WordCloud::Cloud * extCloud, QObject * parent)
  : Appliance::AbstractAppliance(parent)
  , m_extCloud(extCloud)
  , m_scene(new AbstractScene(0))
  , ui(new Ui::WordcloudApplianceElements)
  , m_dummyWidget(new QWidget)
{
    // init UI
    ui->setupUi(m_dummyWidget);

    // set the target scene of the cloud to this
    m_extCloud->setScene(m_scene);

    QPushButton * btn = new QPushButton(tr("Randomize"));
    connect(btn, SIGNAL(clicked()), this, SLOT(slotRandomizeCloud()));

    // configure the appliance
    sceneSet(m_scene);
    sidebarSetWidget(btn);
}

WordcloudAppliance::~WordcloudAppliance()
{
    if (m_extCloud) {
        qWarning("WordcloudAppliance::~WordcloudAppliance: cloud's still here.. take it!");
        m_extCloud->setScene(0);
    }
    delete m_scene;
    delete m_dummyWidget;
    delete ui;
}

WordCloud::Cloud * WordcloudAppliance::takeCloud()
{
    WordCloud::Cloud * cloud = m_extCloud;
    m_extCloud = 0;
    cloud->setScene(0);
    return cloud;
}

WordCloud::Cloud * WordcloudAppliance::cloud() const
{
    return m_extCloud;
}

void WordcloudAppliance::slotRegenCloud()
{
    m_extCloud->regenCloud();
}

void WordcloudAppliance::slotRandomizeCloud()
{
    m_extCloud->randomCloud();
}
