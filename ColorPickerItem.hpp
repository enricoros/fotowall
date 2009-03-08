/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                                                                                                 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __ColorPickerItem_h__
#define __ColorPickerItem_h__

#include <QtGui/QGraphicsItem>
#include <QtGui/QIcon>

class ColorPickerItem : public QObject, public QGraphicsItem {
    Q_OBJECT
    public:
        ColorPickerItem(int width, int height, QGraphicsItem * parent = 0);

        enum Anchor {
            AnchorCenter        = 0x00,
            AnchorLeft          = 0x01,
            AnchorRight         = 0x02,
            AnchorTop           = 0x08,
            AnchorBottom        = 0x10,
            AnchorTopLeft       = AnchorLeft | AnchorTop,
            AnchorBottomLeft    = AnchorLeft | AnchorBottom,
            AnchorTopRight      = AnchorRight | AnchorTop,
            AnchorBottomRight   = AnchorRight | AnchorBottom
        };

        // colors
        void setColor(const QColor & color);
        QColor color() const;
        void setAnimated(bool animated);
        bool animated() const;
        void setAnchor(Anchor anchor);
        Anchor anchor() const;

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    signals:
        void colorChanged(const QColor & color);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

    private:
        void pickColor(const QPoint & mousePos);
        void regenHueSatPixmap();
        void regenValPixmap();
        void updateTransform();

        qreal m_hue;
        qreal m_sat;
        qreal m_val;
        QSize m_size;
        QRect m_hueSatRect;
        QRect m_valRect;
        QPixmap m_hueSatPixmap;
        QPixmap m_valPixmap;
        bool  m_isAnimated;
        class QTimeLine * m_timeLine;
        qreal m_scale;
        Anchor m_anchor;

    private slots:
        void slotAnimateScale(int step);
};

#endif
