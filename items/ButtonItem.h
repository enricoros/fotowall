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

#ifndef __ButtonItem_h__
#define __ButtonItem_h__

#include <QGraphicsItem>
#include <QIcon>
#include <QBrush>
#include <QPointF>

class ButtonItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        ButtonItem(QGraphicsItem * parent, const QBrush & brush, const QIcon & icon);

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
        void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

    Q_SIGNALS:
        void dragging(const QPointF & point);
        void clicked();
        void doubleClicked();

    private:
        QIcon       m_icon;
        QBrush      m_brush;
        QPointF     m_startPos;
};

#endif
