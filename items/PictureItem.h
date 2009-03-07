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
class Frame;
class ButtonItem;
class QGraphicsTextItem;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PictureItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        PictureItem(QGraphicsItem * parent = 0);
        ~PictureItem();

        // photo
        bool loadPhoto(const QString & fileName, bool keepRatio = false, bool setName = false);
        QPixmap renderPhoto(const QSize & size) const;

        // frame
        void setFrame(Frame * frame);

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
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void wheelEvent(QGraphicsSceneWheelEvent * event);
        void keyPressEvent(QKeyEvent * event);
        QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    Q_SIGNALS:
        void deleteMe();
        void raiseMe();
        void backgroundMe();

    private:
        void relayoutContents();
        Frame *     m_frame;
        QString     m_fileName;
        QPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        QSize       m_size;
        ButtonItem *  m_scaleButton;
        ButtonItem *  m_rotateButton;
        ButtonItem *  m_frontButton;
        ButtonItem *  m_deleteButton;
        QGraphicsTextItem * m_textItem;
        QTimer *    m_scaleRefreshTimer;
        bool        m_scaling;

    private Q_SLOTS:
        void slotRotate(const QPointF & controlPoint);
        void slotResize(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers);
        void slotResetAspectRatio();
        void slotResetRotation();
        void slotResizeEnded();
};

#endif
