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

// ### FIXME! bad crossing!
#include "App/FotowallFile.h"
#include "Canvas.h"

#include <QFileInfo>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

CanvasViewContent::CanvasViewContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_canvas(0)
    , m_canvasTaken(false)
{
}

CanvasViewContent::~CanvasViewContent()
{
    if (m_canvasTaken)
        qWarning("CanvasViewContent::~CanvasViewContent: canvas still exported");
    delete m_canvas;
}

bool CanvasViewContent::loadFromFile(const QString & filePath, bool /*keepRatio*/, bool setName)
{
    // ### HACK ahead
    if (!scene() || scene()->views().isEmpty())
        return false;
    QRect viewRect = scene()->views().first()->contentsRect();

    // create a Canvas
    Canvas * canvas = new Canvas(viewRect.size(), this);
    connect(canvas, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintCanvas(const QList<QRectF> &)));
    bool ok = FotowallFile::read(filePath, canvas, false);

    // set the canvas
    m_canvas = canvas;
    m_canvasCachedSize = m_canvas->sceneSize();
    resizeContents(contentRect(), true);
    update();

    // customize the item
    setFrameTextEnabled(setName);
    setFrameText(QFileInfo(filePath).baseName());
    return ok;
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
    // shouldn't paint if canvas is taken.. use a scary red
    if (m_canvasTaken) {
        painter->fillRect(targetRect, Qt::red);
        return;
    }

    // render canvas
    if (m_canvas)
        m_canvas->render(painter, targetRect, m_canvas->sceneRect(), Qt::IgnoreAspectRatio);
}

int CanvasViewContent::contentHeightForWidth(int width) const
{
    // keep aspect ratio of the canvas
    if (m_canvas && m_canvas->width() > 0)
        return (m_canvas->height() * width) / m_canvas->width();
    if (m_canvasCachedSize.width() > 0)
        return (m_canvasCachedSize.height() * width) / m_canvasCachedSize.width();
    return width;
}

QVariant CanvasViewContent::takeResource()
{
    // sanity check
    if (m_canvasTaken) {
        qWarning("CanvasViewContent::takeResource: already taken");
        return QVariant();
    }

    // discard reference
    m_canvasTaken = true;
    Canvas * canvas = m_canvas;
    m_canvas = 0;
    update();
    return qVariantFromValue((void *)canvas);
}

void CanvasViewContent::returnResource(const QVariant & resource)
{
    // sanity checks
    if (!m_canvasTaken)
        qWarning("CanvasViewContent::returnCanvas: not taken");
    if (m_canvas) {
        qWarning("CanvasViewContent::returnCanvas: we already have one canvas, shouldn't return one");
        delete m_canvas;
    }

    // store reference
    m_canvas = static_cast<Canvas *>(qVariantValue<void *>(resource));
    m_canvasCachedSize = m_canvas->sceneSize();
    m_canvasTaken = false;
    resizeContents(contentRect(), true);
    update();
}

void CanvasViewContent::slotRepaintCanvas(const QList<QRectF> &)
{
    update();
}
