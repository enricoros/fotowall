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

#ifndef __AbstractContent_h__
#define __AbstractContent_h__

#include <QGraphicsItem>
#include <QObject>
#include <QDomElement>
class AbstractProperties;
class ButtonItem;
class CornerItem;
class Frame;
class MirrorItem;
class QGraphicsTextItem;
class QPointF;


/// \brief Base class of Canvas Item (with lots of gadgets!)
class AbstractContent : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        AbstractContent(QGraphicsScene * scene, QGraphicsItem * parent = 0, bool noRescale = false);
        virtual ~AbstractContent();

        // size
        QRect contentsRect() const;
        void resizeContents(const QRect & rect, bool keepRatio = false);
        void resetContentsRatio();
        void delayedDirty(int ms = 400);

        // frame (and frame text)
        void setFrame(Frame * frame);
        quint32 frameClass() const;
        void setFrameTextEnabled(bool enabled);
        bool frameTextEnabled() const;
        void setFrameText(const QString & text);
        QString frameText() const;
        void addButtonItem(ButtonItem * buttonItem);

        // mirror
        void setMirrorEnabled(bool enabled);
        bool mirrorEnabled() const;

        void setRotation(double pan, double tilt, double roll);

        // misc
        void ensureVisible(const QRectF & viewportRect);
        bool beingTransformed() const;

        // may be reimplemented by subclasses
        virtual bool fromXml(QDomElement & parentElement);
        virtual void toXml(QDomElement & parentElement) const;
        virtual QPixmap renderAsBackground(const QSize & size, bool keepAspect = false) const;

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    Q_SIGNALS:
        void gfxChange();
        void configureMe(const QPoint & scenePoint);
        void changeStack(int opcode);
        void backgroundMe();
        void deleteItem();

    protected:
        // useful to subclasses
        void GFX_CHANGED() const;

        // may be reimplemented by subclasses
        virtual int contentHeightForWidth(int width) const;
        virtual bool contentOpaque() const;

        // ::QGraphicsItem
        void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void keyPressEvent(QKeyEvent * event);
        QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    protected Q_SLOTS:
        void slotConfigure();
        void slotStackFront();
        void slotStackRaise();
        void slotStackLower();
        void slotStackBack();
        void slotSaveAs();

    private:
        void createCorner(Qt::Corner corner, bool noRescale);
        void layoutChildren();
        void applyTransformations();
        QRect               m_contentsRect;
        QRectF              m_frameRect;
        Frame *             m_frame;
        QGraphicsTextItem * m_frameTextItem;
        QList<ButtonItem *> m_controlItems;
        QList<CornerItem *> m_cornerItems;
        bool                m_dirtyTransforming;
        QTimer *            m_transformRefreshTimer;
        QTimer *            m_gfxChangeTimer;
        MirrorItem *        m_mirrorItem;
        double m_xRotationAngle, m_yRotationAngle, m_zRotationAngle;

    private Q_SLOTS:
        void slotPerspective(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers);
        void slotDirtyEnded();
};

#endif
