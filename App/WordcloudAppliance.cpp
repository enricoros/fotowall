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

    QWidget * sideBar = new QWidget();
    sideBar->setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(QPalette::Window, Qt::lightGray);
    sideBar->setPalette(pal);
    QVBoxLayout * lay = new QVBoxLayout(sideBar);

    QPushButton * btn1 = new QPushButton(tr("Regen"), sideBar);
    connect(btn1, SIGNAL(clicked()), this, SLOT(slotRegenCloud()));
    lay->addWidget(btn1);

    QPushButton * btn2 = new QPushButton(tr("Randomize"), sideBar);
    connect(btn2, SIGNAL(clicked()), this, SLOT(slotRandomizeCloud()));
    lay->addWidget(btn2);

    lay->addStretch(10);

    // configure the appliance
    sceneSet(m_scene);
    sidebarSetWidget(sideBar);

    // #temp
    foreach (QGraphicsItem * item, m_scene->items())
        item->setPos(item->pos() + QPointF(500,200));
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
