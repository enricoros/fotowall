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

#ifndef __HighLightItem_h__
#define __HighLightItem_h__

#include <QObject>
#include <QGraphicsItem>
#include <QBasicTimer>

class HighlightItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        HighlightItem();

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

        // ::QObject
        void timerEvent(QTimerEvent * event);

    private:
        void resize(double rad);
        QBasicTimer m_timer;
        int m_phase;
        double m_radius;
};

#endif
