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

#ifndef __TextContent_h__
#define __TextContent_h__

#include "AbstractContent.h"
class QGraphicsTextItem;

/// \brief TODO
class TextContent : public AbstractContent
{
    public:
        TextContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~TextContent();

        // ::AbstractContent
        void save(QDataStream & data) const;
        bool restore(QDataStream & data);

        // ::QGraphicsItem
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        QGraphicsTextItem * m_textItem;
};

#endif
