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

#ifndef __PixmapContent_h__
#define __PixmapContent_h__

#include "AbstractContent.h"
class CPixmap;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PixmapContent : public AbstractContent
{
    Q_OBJECT
    public:
        PixmapContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~PixmapContent();

        void setPixmp(const QPixmap & pixmap, bool keepRatio = false);

        // ::AbstractContent
        QPixmap renderAsBackground(const QSize & size) const;
        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        QPixmap m_pixmap;
        QPixmap m_cachedPixmap;
};

#endif
