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

#ifndef __MirrorItem_h__
#define __MirrorItem_h__

#include "AbstractContentItem.h"

/**
    \brief Mirrors a transformed PictureItem
*/
class MirrorItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        MirrorItem(QGraphicsItem * sourceItem, QGraphicsItem * parent = 0);
        ~MirrorItem();

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    public Q_SLOTS:
        void sourceUpdated();

    private:
        QGraphicsItem * m_source;
        QRectF m_boundingRect;
        QPixmap m_pixmap;
};

#endif
