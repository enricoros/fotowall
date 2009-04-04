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

#ifndef __PictureItem_h__
#define __PictureItem_h__

#include "AbstractContentItem.h"
#include "CPixmap.h"
class QGraphicsTextItem;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PictureItem : public AbstractContentItem
{
    Q_OBJECT
    public:
        PictureItem(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~PictureItem();

        bool loadPhoto(const QString & fileName, bool keepRatio = false, bool setName = false);
        void setEffect(int effectClass);
        QPixmap renderPhoto(const QSize & size) const;

        // ::AbstractContentItem
        void save(QDataStream & data) const;
        bool restore(QDataStream & data);

        // ::QGraphicsItem
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        QString     m_fileName;
        CPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        bool        m_opaquePhoto;
        QGraphicsTextItem * m_textItem;

    protected Q_SLOTS:
        void slotFlipHorizontally();
        void slotFlipVertically();
};

#endif
