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

#include "CanvasViewContent.h"

#include "App/XmlRead.h"
#include "Canvas.h"

#include <QFileInfo>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

CanvasViewContent::CanvasViewContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_canvas(0)
{
}

bool CanvasViewContent::loadCanvas(const QString & filePath, bool /*keepRatio*/, bool setName)
{
    // ### HACK ahead
    if (!scene() || scene()->views().isEmpty())
        return false;
    QRect viewRect = scene()->views().first()->contentsRect();

    // create a Canvas
    m_canvas = new Canvas(viewRect.size(), this);
    connect(m_canvas, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintCanvas(const QList<QRectF> &)));
    bool ok = XmlRead::read(filePath, m_canvas);

    // customize the item
    setFrameTextEnabled(setName);
    setFrameText(QFileInfo(filePath).baseName());
    return ok;
}

QWidget * CanvasViewContent::createPropertyWidget()
{
    return 0;
}

bool CanvasViewContent::fromXml(QDomElement & /*parentElement*/)
{
    return false;
}

void CanvasViewContent::toXml(QDomElement & /*parentElement*/) const
{
}

void CanvasViewContent::drawContent(QPainter * painter, const QRect & targetRect)
{
    // TODO: see if it paints when the scene is not displayed...
    if (m_canvas)
        m_canvas->render(painter, targetRect, m_canvas->sceneRect(), Qt::IgnoreAspectRatio);
    else
        painter->fillRect(targetRect, Qt::red);
}

bool CanvasViewContent::contentOpaque() const
{
    return false;
}

#include "App/App.h"
#include "App/MainWindow.h"
void CanvasViewContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    setSelected(false);
    App::mainWindow->editCanvas(m_canvas);
}

void CanvasViewContent::slotRepaintCanvas(const QList<QRectF> & /*exposed*/)
{
    update();
}
