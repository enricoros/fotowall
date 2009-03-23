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

#include <QGraphicsItem>
#include <QBrush>
#include <QIcon>
#include <QPointF>
#include <QObject>
#include "CPixmap.h"

class ButtonItem;
class Frame;
class MirrorItem;
class QGraphicsTextItem;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PictureItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        PictureItem(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~PictureItem();

        // TEMP: replace with globals
        static bool s_defaultMirrorEnabled;

        // photo
        bool loadPhoto(const QString & fileName, bool keepRatio = false, bool setName = false);
        QPixmap renderPhoto(const QSize & size) const;

        // frame
        void setFrame(Frame * frame);
        quint32 frameClass() const;

        // mirror
        bool mirrorEnabled() const;
        void setMirrorEnabled(bool enabled);

        // effect
        void setEffect(int effectClass);

        // save/restore
        void save(QDataStream & data) const;
        bool restore(QDataStream & data);

        // misc
        void ensureVisible(const QRectF & viewportRect);

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void wheelEvent(QGraphicsSceneWheelEvent * event);
        void keyPressEvent(QKeyEvent * event);
        QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    Q_SIGNALS:
        void gfxChange();
        void configureMe(const QPoint & scenePoint);
        void changeStack(int);
        void backgroundMe();
        void deleteMe();

    private:
        void relayoutContents();
        Frame *     m_frame;
        QString     m_fileName;
        CPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        bool        m_opaquePhoto;
        QSize       m_size;
        QList<ButtonItem *> m_controlItems;
        ButtonItem * m_scaleButton;
        ButtonItem * m_rotateButton;
        QGraphicsTextItem * m_textItem;
        MirrorItem * m_mirrorItem;
        QTimer *    m_gfxChangeSignalTimer;
        QTimer *    m_scaleRefreshTimer;
        bool        m_scaling;

    public Q_SLOTS:
        void slotFlipHorizontally();
        void slotFlipVertically();
        void slotStackFront();
        void slotStackRaise();
        void slotStackLower();
        void slotStackBack();
        void slotSave();

    private Q_SLOTS:
        void slotConfigure();
        void slotRotate(const QPointF & controlPoint);
        void slotResize(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers);
        void slotResetAspectRatio();
        void slotResetRotation();
        void slotResizeEnded();
};

#endif
