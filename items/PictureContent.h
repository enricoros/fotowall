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

#ifndef __PictureContent_h__
#define __PictureContent_h__

#include "AbstractContent.h"
class CPixmap;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PictureContent : public AbstractContent
{
    Q_OBJECT
    public:
        PictureContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~PictureContent();

        bool loadPhoto(const QString & fileName, bool keepRatio = false, bool setName = false);
        void setEffect(int effectClass);

        // ::AbstractContent
        void save(QDataStream & data) const;
        bool restore(QDataStream & data);
        QPixmap renderAsBackground(const QSize & size) const;
        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        QString     m_fileName;
        CPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        bool        m_opaquePhoto;

    private Q_SLOTS:
        void slotFlipHorizontally();
        void slotFlipVertically();
};

#endif
