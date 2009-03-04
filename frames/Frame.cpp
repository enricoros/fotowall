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

#include "Frame.h"
#include <QRectF>

QRect Frame::contentsRect(const QRect & frameRect) const
{
    return frameRect.adjusted(5, 5, -5, -30);
}

bool Frame::clipContents() const
{
    return false;
}

QPainterPath Frame::contentsClipPath(const QRect & frameRect) const
{
    QPainterPath path;
    path.addRect(contentsRect(frameRect));
    return path;
}

bool Frame::isShaped() const
{
    return false;
}

QPainterPath Frame::frameShape(const QRect & frameRect) const
{
    QPainterPath path;
    path.addRect(frameRect);
    return path;
}

QRect Frame::buttonsRect(const QRect & geometry)
{
    return QRect(geometry.right() - 40, geometry.bottom() - 20, 30, 10);
}
