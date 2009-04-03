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

#ifndef __AbstractContentItem_h__
#define __AbstractContentItem_h__

#include <QGraphicsItem>
#include <QBrush>
#include <QIcon>
#include <QPointF>
#include <QObject>

class ButtonItem;
class Frame;
class MirrorItem;

/**
    \brief Transformable picture, with lots of gadgets
*/
class AbstractContentItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        AbstractContentItem(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~AbstractContentItem();

        // misc
        void ensureVisible(const QRectF & viewportRect);
        bool beingTransformed() const;

        // frame
        void setFrame(Frame * frame);
        quint32 frameClass() const;

        // mirror
        bool mirrorEnabled() const;
        void setMirrorEnabled(bool enabled);

        // save/restore
        virtual void save(QDataStream & data) const;
        virtual bool restore(QDataStream & data);

        // ::QGraphicsItem
        virtual QRectF boundingRect() const;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        virtual void dropEvent(QGraphicsSceneDragDropEvent * event);
        virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent * event);
        virtual void keyPressEvent(QKeyEvent * event);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    Q_SIGNALS:
        void gfxChange();
        void configureMe(const QPoint & scenePoint);
        void changeStack(int);
        void backgroundMe();
        void deleteMe();

    protected:
        void GFX_CHANGED();
        void slotResetAspectRatio();
        Frame *     m_frame;

    private:
        virtual void relayoutContents();
        QSize       m_size;
        QList<ButtonItem *> m_controlItems;
        ButtonItem * m_scaleButton;
        ButtonItem * m_rotateButton;
        MirrorItem * m_mirrorItem;
        QTimer *    m_gfxChangeSignalTimer;
        QTimer *    m_transformRefreshTimer;
        bool        m_transforming;

    public Q_SLOTS:
        void slotStackFront();
        void slotStackRaise();
        void slotStackLower();
        void slotStackBack();
        void slotSave();
        virtual void slotConfigure();

    private Q_SLOTS:
        void slotRotate(const QPointF & controlPoint);
        void slotResize(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers);
        void slotResetRotation();
        void slotResizeEnded();
};

#endif
