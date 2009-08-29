/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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

#ifndef __HelpItem_h__
#define __HelpItem_h__

#include "AbstractDisposeable.h"
class Frame;

class HelpItem : public AbstractDisposeable
{
    Q_OBJECT
    public:
        HelpItem(QGraphicsItem * parent = 0);
        ~HelpItem();

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);

    Q_SIGNALS:
        void closeMe();

    private:
        Frame * m_frame;
};

#endif
