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

#ifndef __MirrorItem_h__
#define __MirrorItem_h__

#include "AbstractDisposeable.h"

/**
    \brief Mirrors a transformed PictureContent
*/
class MirrorItem : public AbstractDisposeable
{
    Q_OBJECT
    public:
        MirrorItem(QGraphicsItem * sourceItem, QGraphicsItem * parent = 0);
        ~MirrorItem();

        // ::AbstractDisposeable
        //void dispose();

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    public Q_SLOTS:
        void sourceMoved();
        void sourceChanged();

    private:
        QGraphicsItem * m_source;
        QRectF m_boundingRect;
        QPixmap m_pixmap;
        bool m_dirty;
};

#endif
