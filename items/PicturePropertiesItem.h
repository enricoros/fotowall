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

#ifndef __PicturePropertiesItem_h__
#define __PicturePropertiesItem_h__

#include <QGraphicsProxyWidget>
#include <QBasicTimer>
#include "PictureItem.h"
class Frame;
class QListWidgetItem;

namespace Ui { class PicturePropertiesItem; }

class PicturePropertiesItem : public QGraphicsProxyWidget {
    Q_OBJECT
    public:
        PicturePropertiesItem(PictureItem * pictureItem, QGraphicsItem * parent = 0);
        ~PicturePropertiesItem();

        // set which item to edit
        void setPictureItem(PictureItem * pictureItem);
        PictureItem * pictureItem() const;

        // load from/apply to the properties of the current valid item
        void loadProperties();
        void applyProperties();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        void timerEvent(QTimerEvent *);

    private:
        Ui::PicturePropertiesItem * m_ui;
        PictureItem *               m_pictureItem;
        Frame *                     m_frame;
        int                         m_aniStep;
        bool                        m_aniDirection;
        QBasicTimer                 m_aniTimer;

    private slots:
        void slotFrameSelected(QListWidgetItem * item);
        void slotClickedOk();
        void slotClickedCancel();
};

#endif
