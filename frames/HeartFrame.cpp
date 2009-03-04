/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "HeartFrame.h"
#include <QLinearGradient>
#include <QPainter>

static QPainterPath heartPath(const QRect & r)
{
    double left = r.left(),
           top = r.top(),
           height = r.height(),
           width = r.width(),
           p1x = left + width / 2,
           p1y = top + height / 3,
           p2x = p1x,
           p2y = top + height,
           h4 = height / 4,
           skew = 0, //width / 8
           offs = height / 10;
    QPainterPath path;
    path.moveTo( p1x, p1y - offs );
    path.cubicTo( p1x, top, left + width, top, left + width, p1y );
    path.cubicTo( left + width, p1y + h4, p2x + skew, p2y - h4, p2x, p2y );
    path.cubicTo( p2x + skew, p2y - h4, left, p1y + h4, left, p1y );
    path.cubicTo( left, top, p1x, top, p1x, p1y - offs );
    return path;
}

QRect HeartFrame::contentsRect(const QRect & frameRect) const
{
    int fw = frameRect.width() / 10;
    int fh = frameRect.height() / 10;
    return frameRect.adjusted(fw, fh, -fw, -fh);
}

bool HeartFrame::clipContents() const
{
    return true;
}

QPainterPath HeartFrame::contentsClipPath(const QRect & frameRect) const
{
    return heartPath(contentsRect(frameRect));
}

bool HeartFrame::isShaped() const
{
    return true;
}

QPainterPath HeartFrame::frameShape(const QRect & frameRect) const
{
    return heartPath(frameRect);
}

/*QRect HeartFrame::buttonsRect(const QRect & geometry)
{
}*/

void HeartFrame::paint(QPainter * painter, const QRect & frameRect)
{
    QLinearGradient lg(0, frameRect.top(), 0, frameRect.height() / 2);
    lg.setColorAt(0.0, Qt::red);
    lg.setColorAt(1.0, QColor(255,50,50, 200));

    QPainterPath path = heartPath(frameRect);
    painter->setPen(QPen(Qt::black, 1));
    painter->setBrush(lg);
    painter->drawPath(path);
}
