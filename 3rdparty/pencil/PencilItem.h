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

#ifndef __PencilItem_h__
#define __PencilItem_h__

#include <QGraphicsItem>
#include <QLineF>
#include <QList>
#include <QPainterPath>
#include <QRectF>

#if QT_VERSION >= 0x040600

// the pencil item is present
#define HAVE_PENCIL_ITEM

class PencilItem : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(qreal alpha READ alpha WRITE setAlpha)
    public:
        PencilItem(const QString & svgFileName, QGraphicsItem * parent = 0);

        // :QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    private:
        // properties
        qreal alpha() const;
        void setAlpha(qreal alpha);

        struct Step {
            bool move;
            QPointF point;
        };

        QRectF m_rect;
        qreal m_alpha;
        QList<Step> m_steps;

};
#endif

#endif
