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

#include "WordCloudContent.h"

#include "App/XmlRead.h"
#include "WordCloud/WordScanner.h"
#include "Canvas.h"

#include <QGraphicsScene>
#include <QPainter>

WordCloudContent::WordCloudContent(QGraphicsScene * scene, QGraphicsItem * parent)
  : AbstractContent(scene, parent, false)
  , m_cloudScene(new QGraphicsScene)
  , m_cloud(new WordCloud::Cloud)
{
    connect(m_cloudScene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintScene(const QList<QRectF> &)));
    m_cloud->setScene(m_cloudScene);

    // temporarily get words
    WordCloud::Scanner scanner;
    //scanner.addFromFile("/alchimia");
    scanner.addFromString(tr("Welcome to WordCloud. Change options on the sidebar."));
    WordCloud::WordList list = scanner.takeWords();
    WordCloud::WordList::iterator wIt = list.begin();
    int ccc = list.size() + 1;
    while (wIt != list.end()) {
        wIt->count = ccc--;
        ++wIt;
    }
    m_cloud->newCloud(list);
}

WordCloud::Cloud * WordCloudContent::cloud() const
{
    return m_cloud;
}

QWidget * WordCloudContent::createPropertyWidget()
{
    return 0;
}

bool WordCloudContent::fromXml(QDomElement & /*parentElement*/)
{
    return false;
}

void WordCloudContent::toXml(QDomElement & /*parentElement*/) const
{
}

QPixmap WordCloudContent::renderContent(const QSize & /*size*/, Qt::AspectRatioMode /*ratio*/) const
{
    return QPixmap(100, 100);
}

bool WordCloudContent::contentOpaque() const
{
    return false;
}

#include "App/App.h"
#include "App/MainWindow.h"
void WordCloudContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    App::mainWindow->editWordcloud(m_cloud);
}

void WordCloudContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    m_cloudScene->render(painter, contentsRect(), m_cloudScene->sceneRect(), Qt::KeepAspectRatio);
}

void WordCloudContent::slotRepaintScene(const QList<QRectF> & /*exposed*/)
{
    update();
}
