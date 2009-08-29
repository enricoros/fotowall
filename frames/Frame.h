/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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

#include <QGraphicsItem>
#include <QList>
#include <QPainterPath>
#include <QPainter>
#include <QRect>
#include <QSize>
#include "items/ButtonItem.h"
class QPainter;

/**
    \brief Paints the frame, selects the clipping region and controls position.
    G: geometry
    P: painting
*/
class Frame {
    public:
        // class identification
        virtual quint32 frameClass() const = 0;
        enum { NoFrame = 0 };

        // G: frame geometry
        virtual QRect frameRect(const QRect & contentsRect) const;

        // G: contents clipping
        virtual bool clipContents() const;
        virtual QPainterPath contentsClipPath(const QRect & contentsRect) const;

        // G: frame shape
        virtual bool isShaped() const;
        virtual QPainterPath frameShape(const QRect & frameRect) const;

        // G: layouting sub-items
        virtual void layoutButtons(QList<ButtonItem *> buttons, const QRect & frameRect) const = 0;
        virtual void layoutText(QGraphicsItem * textItem, const QRect & frameRect) const = 0;

        // P: painting
        virtual void paint(QPainter * painter, const QRect & geometry, bool selected, bool opaqueContents) = 0;
        virtual QPixmap preview(int width = 32, int height = 32);

        // unbreak stuff
        virtual ~Frame();
};

#endif
