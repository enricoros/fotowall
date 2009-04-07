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
#include "PictureContent.h"
class Frame;
class PixmapButton;
class QAbstractButton;
class QListWidgetItem;

namespace Ui { class PicturePropertiesItem; }

class PicturePropertiesItem : public QGraphicsProxyWidget {
    Q_OBJECT
    public:
        PicturePropertiesItem(PictureContent * pictureContent, QGraphicsItem * parent = 0);
        ~PicturePropertiesItem();

        // the watched item
        PictureContent * pictureContent() const;

        // load from/apply to the properties of the current valid item
        void loadProperties();

        // load the list of available effects
        void loadEffectsList();

        // misc
        void keepInBoundaries(const QRect & rect);
        void animateClose();

    Q_SIGNALS:
        void closed();
        void applyAll(quint32 frameClass, bool mirrored);
        void applyEffectToAll(int effectClass);

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        void resizeEvent(QGraphicsSceneResizeEvent * event);
        void timerEvent(QTimerEvent *);

    private:
        Ui::PicturePropertiesItem * m_ui;
        PictureContent *            m_pictureContent;
        PixmapButton *              m_closeButton;
        Frame *                     m_frame;
        int                         m_aniStep;
        bool                        m_aniDirection;
        QBasicTimer                 m_aniTimer;

    private Q_SLOTS:
        void slotEffectSelected(QListWidgetItem * item);
        void slotFrameSelected(QListWidgetItem * item);
        void slotToggleMirror(bool enabled);
        void slotClose(QAbstractButton * button);
};

#endif
