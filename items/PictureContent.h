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
class CEffect;
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
        void addEffect(const CEffect & effect);

        QString getFilePath() const;

        // ::AbstractContent
        QPixmap renderAsBackground(const QSize & size) const;
        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

        void flipH();
        void flipV();

        CPixmap * getCPixmap() const;
    private:
        QString     m_filePath;
        CPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        bool        m_opaquePhoto;

    Q_SIGNALS:
        void flipHorizontally();
        void flipVertically();
};

#endif
