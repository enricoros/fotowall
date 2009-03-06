/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ButtonItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

// from FotoWall.cpp
extern bool globalExportingFlag;

ButtonItem::ButtonItem(QGraphicsItem * parent, const QBrush & brush, const QIcon & icon)
    : QGraphicsItem(parent)
    , m_icon(icon)
    , m_brush(brush)
{
    setAcceptsHoverEvents(true);
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

QRectF ButtonItem::boundingRect() const
{
    return QRectF(-8, -8, 16, 16);
}

void ButtonItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
    if (globalExportingFlag)
        return;
    bool over = option->state & QStyle::State_MouseOver;
    if (over) {
        if (m_startPos.isNull())
            painter->fillRect(boundingRect().adjusted(-1, -1, 1, 1), m_brush);
        else
            painter->fillRect(boundingRect().adjusted(-1, -1, 1, 1), Qt::white);
    }
    if (!m_icon.isNull())
        m_icon.paint(painter, boundingRect().toRect(), Qt::AlignCenter, over ? QIcon::Active : QIcon::Normal);
}

void ButtonItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    m_startPos = event->scenePos();
    update();
}

void ButtonItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_startPos.isNull())
        return;
    event->accept();
    emit dragging(event->scenePos());
}

void ButtonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    m_startPos = QPointF();
    update();
}

void ButtonItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    emit reset();
}

