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

#include "WordcloudContent.h"

#include "Wordcloud/Scanner.h"
#include "Canvas.h"

#include <QGraphicsScene>
#include <QFileDialog>
#include <QPainter>

WordcloudContent::WordcloudContent(QGraphicsScene * scene, QGraphicsItem * parent)
  : AbstractContent(scene, parent, false)
  , m_cloudScene(new QGraphicsScene)
  , m_cloud(new Wordcloud::Cloud)
{
    connect(m_cloudScene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintScene(const QList<QRectF> &)));
    m_cloud->setScene(m_cloudScene);

    // temporarily get words
    Wordcloud::Scanner scanner;
    QString fileName = QFileDialog::getOpenFileName(0, tr("Select a text file"));
    if (fileName.isEmpty()) {
        scanner.addFromString(tr("Welcome to Wordcloud. Change options on the sidebar."));
        Wordcloud::WordList list = scanner.takeWords();
        Wordcloud::WordList::iterator wIt = list.begin();
        int ccc = list.size() + 1;
        while (wIt != list.end()) {
            wIt->count = ccc--;
            ++wIt;
        }
        m_cloud->newCloud(list);
    } else {
        scanner.addFromFile(fileName);
        m_cloud->newCloud(scanner.takeWords());
    }
}

Wordcloud::Cloud * WordcloudContent::cloud() const
{
    return m_cloud;
}

QWidget * WordcloudContent::createPropertyWidget()
{
    return 0;
}

bool WordcloudContent::fromXml(QDomElement & contentElement)
{
    AbstractContent::fromXml(contentElement);

    // ### load wordcloud properties
    return false;
}

void WordcloudContent::toXml(QDomElement & contentElement) const
{
    AbstractContent::toXml(contentElement);
    contentElement.setTagName("wordcloud");

    // ### save all wordclouds
}

void WordcloudContent::drawContent(QPainter * painter, const QRect & targetRect)
{
    m_cloudScene->render(painter, targetRect, m_cloudScene->sceneRect(), Qt::KeepAspectRatio);
}

bool WordcloudContent::contentOpaque() const
{
    return false;
}

void WordcloudContent::slotRepaintScene(const QList<QRectF> & /*exposed*/)
{
    update();
}
