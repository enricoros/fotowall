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

#include "HomeAppliance.h"

#include "App.h"
#include "HomeScene.h"
#include "Settings.h"
#include "UrlHistoryBox.h"
#include "Workflow.h"


/** Home Appliance **/

HomeAppliance::HomeAppliance(QObject *parent)
    : PlugGui::AbstractAppliance(parent)
    , m_scene(0)
    , m_historyBox(0)
{
    // create and set the scene
    m_scene = new HomeScene;
    connect(m_scene, SIGNAL(keyPressed(int)), this, SLOT(slotSceneKeyPressed(int)));
    connect(m_scene, SIGNAL(startCanvas()), this, SLOT(slotStartCanvas()));
    connect(m_scene, SIGNAL(startWordcloud()), this, SLOT(slotStartWordcloud()));
    connect(m_scene, SIGNAL(startWizard()), this, SLOT(slotStartWizard()));
    sceneSet(m_scene);

    // create the History Box, if enough history
    QList<QUrl> recentUrls = App::settings->recentFotowallUrls();
    if (!recentUrls.isEmpty()) {
        m_historyBox = new UrlHistoryBox(recentUrls);
        m_historyBox->setTitle(tr("RECENT FILES"));
        m_historyBox->setBorderFlags(0x0000);
        m_historyBox->setCheckable(false);
        QPalette pal;
        pal.setBrush(QPalette::Window, QColor(255, 255, 255, 128));
        m_historyBox->setPalette(pal);
        m_historyBox->setAutoFillBackground(true);
        connect(m_historyBox, SIGNAL(urlClicked(QUrl)), this, SLOT(slotLoadCanvas(const QUrl &)));
        topbarAddWidget(m_historyBox);
    }
}

HomeAppliance::~HomeAppliance()
{
    delete m_historyBox;
    delete m_scene;
}

void HomeAppliance::slotSceneKeyPressed(int qtKey)
{
    // pressed a number, activate relative Url
    if (qtKey >= Qt::Key_1 && qtKey <= Qt::Key_9) {
        QUrl url = m_historyBox->urlForEntry(qtKey - Qt::Key_0 - 1);
        if (!url.isEmpty())
            slotLoadCanvas(url);
    }
}

void HomeAppliance::slotLoadCanvas(const QUrl & url)
{
    // handle fotowall files
    if (App::isFotowallFile(url.toString())) {
        App::workflow->loadCanvas(url.toString());
        return;
    }

    // handle other files
    qWarning("HomeAppliance::slotLoadCanvas: don't know how to load URL '%s'", qPrintable(url.toString()));
}

void HomeAppliance::slotStartCanvas()
{
    App::workflow->startCanvas();
}

void HomeAppliance::slotStartWordcloud()
{
    App::workflow->startWordcloud();
}

void HomeAppliance::slotStartWizard()
{
    App::workflow->startWizard();
}
