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
}

#include "App/App.h"
#include "App/MainWindow.h"
void WordCloudContent::stackEditor()
{
    App::mainWindow->stackWordCloud(m_cloud);
    update();
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

void WordCloudContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    stackEditor();
}

void WordCloudContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    m_cloudScene->render(painter, boundingRect(), m_cloudScene->sceneRect(), Qt::IgnoreAspectRatio);
}

void WordCloudContent::slotRepaintScene(const QList<QRectF> & /*exposed*/)
{
    update();
}
