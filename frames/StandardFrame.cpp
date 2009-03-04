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

#include "StandardFrame.h"
#include <QLinearGradient>
#include <QPainter>

void StandardFrame::paint(QPainter * painter, const QRect & frameRect)
{
    //painter->fillRect(boundingRect(), QColor(0,0,0,64));
    QLinearGradient lg(0, frameRect.top(), 0, frameRect.height() / 2);
    lg.setColorAt(0.0, QColor(128,128,128, 200));
    lg.setColorAt(1.0, QColor(255,255,255, 200));
    painter->fillRect(frameRect, lg);
    //painter->fillRect(boundingRect().adjusted(5, 5, -5, -5), lg);
}
