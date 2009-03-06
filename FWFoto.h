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

#ifndef __FWFoto_h__
#define __FWFoto_h__

#include <QGraphicsItem>
#include <QBrush>
#include <QIcon>
#include <QPointF>
#include <QObject>
class Frame;
class FWButton;
class QGraphicsTextItem;

// resizable "photo" item
class FWFoto : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        FWFoto(QGraphicsItem * parent = 0);
        ~FWFoto();

        // photo
        void loadPhoto(const QString & fileName, bool keepRatio = false, bool setName = false);

        // frame
        void setFrame(Frame * frame);

        // save/restore
        void save(QDataStream & data) const;
        void restore(QDataStream & data);

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
        void wheelEvent(QGraphicsSceneWheelEvent * event);
        void keyPressEvent(QKeyEvent * event);

    Q_SIGNALS:
        void deletePressed();

    private:
        void relayoutContents();
        Frame *     m_frame;
        QString     m_fileName;
        QPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        QSize       m_size;
        FWButton *  m_scaleButton;
        FWButton *  m_rotateButton;
        QGraphicsTextItem * m_textItem;
        QTimer *    m_scaleRefreshTimer;
        bool        m_scaling;

    private Q_SLOTS:
        void slotResize(const QPointF & controlPoint);
        void slotRotate(const QPointF & controlPoint);
        void slotResetAspectRatio();
        void slotResetRotation();
        void slotResizeEnded();
};

class FWButton : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        FWButton( FWFoto * parent, const QBrush & brush, const QIcon & icon );

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
        void mouseMoveEvent( QGraphicsSceneMouseEvent * event );
        void mousePressEvent( QGraphicsSceneMouseEvent * event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event );

    Q_SIGNALS:
        void dragging(const QPointF & point);
        void reset();

    private:
        FWFoto *    m_parent;
        QIcon       m_icon;
        QBrush      m_brush;
        QPointF     m_startPos;
};

#endif
