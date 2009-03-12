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

#include "HighlightItem.h"
#include <QPainter>
#include <QTimerEvent>
#include <math.h>

HighlightItem::HighlightItem()
    : QGraphicsItem()
    , m_phase(0)
    , m_radius(1)
{
    m_timer.start(30, this);
}

QRectF HighlightItem::boundingRect() const
{
    return QRectF(-m_radius, -m_radius, 2 * m_radius, 2 * m_radius);
}

void HighlightItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    int alpha = qBound(0, (100 - (int)m_radius) * 3, 255);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor(255, 255, 255, alpha), 3));
    painter->drawEllipse(boundingRect().adjusted(1, 1, -1, -1));
}

void HighlightItem::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_phase++;

        // resize
        prepareGeometryChange();
        m_radius = sqrt((m_phase * 200) % 10000);
        update();

        // stop after 100 steps
        if (m_phase > 100) {
            m_timer.stop();
            deleteLater();
        }
    }
    QObject::timerEvent(event);
}
