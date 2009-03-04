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

#ifndef __Frame_h__
#define __Frame_h__

#include <QPainterPath>
#include <QRect>
class QPainter;

/**
    \brief Paints the frame, selects the clipping region and controls position.
    G: geometry
    P: painting
*/
class Frame {
    public:
        // G: contents geometry
        virtual QRect contentsRect(const QRect & frameRect) const;

        // G: contents clipping
        virtual bool clipContents() const;
        virtual QPainterPath contentsClipPath(const QRect & frameRect) const;

        // G: frame shape
        virtual bool isShaped() const;
        virtual QPainterPath frameShape(const QRect & frameRect) const;

        // G: button placement
        virtual QRect buttonsRect(const QRect & geometry);

        // P: painting
        virtual void paint(QPainter * painter, const QRect & geometry) = 0;
};

#endif
