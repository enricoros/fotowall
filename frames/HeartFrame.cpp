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
           skew = 0 * width / 20,
           offs = height / 20;
    QPainterPath path;
    path.moveTo( p1x, p1y - offs );
    path.cubicTo( p1x, top, left + width, top, left + width, p1y );
    path.cubicTo( left + width, p1y + h4, p2x + skew, p2y - h4, p2x, p2y );
    path.cubicTo( p2x + skew, p2y - h4, left, p1y + h4, left, p1y );
    path.cubicTo( left, top, p1x, top, p1x, p1y - offs );
    return path;
}

QSize HeartFrame::sizeForContentsRatio(int width, qreal ratio) const
{
    return StandardFrame::sizeForContentsRatio(width, ratio);
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

void HeartFrame::layoutButtons(QList<ButtonItem *> buttons, const QRect & frameRect) const
{
    double x[] = { 0.92, 0.85 };
    double y[] = { 0.22, 0.16 };
    for ( int i = 0; i < buttons.size() && i < 2; i++ ) {
        QGraphicsItem * button = buttons[ i ];
        button->setPos( frameRect.left() + (int)((float)frameRect.width() * x[i]),
                        frameRect.top() + (int)((float)frameRect.height() * y[i]));
    }
}

void HeartFrame::paint(QPainter * painter, const QRect & frameRect, bool /*opaqueContents*/)
{
    QLinearGradient lg(0, frameRect.top(), frameRect.width() / 8, frameRect.height() / 2);
    lg.setColorAt(0.0, QColor(196,00,00));
    lg.setColorAt(0.3, Qt::red);
    lg.setColorAt(1.0, QColor(128,00,00));

    QPainterPath path = heartPath(frameRect);
    painter->setPen(QPen(QColor(64, 0, 0), 1.0));
    painter->setBrush(lg);
    painter->drawPath(path);
}
