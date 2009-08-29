/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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

#include "AbstractDisposeable.h"
#include <QDomElement>
#include "3rdparty/enricomath.h"
class AbstractConfig;
class ButtonItem;
class CornerItem;
class Frame;
class MirrorItem;
class QGraphicsTextItem;
class QPointF;


/// \brief Base class of Canvas Item (with lots of gadgets!)
class AbstractContent : public AbstractDisposeable
{
    Q_OBJECT
    public:
        AbstractContent(QGraphicsScene * scene, QGraphicsItem * parent = 0, bool noRescale = false);
        virtual ~AbstractContent();

        // ::AbstractDisposeable
        void dispose();

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

        // rotation
        void setRotation(double angle, Qt::Axis axis);
        double rotation(Qt::Axis axis) const;

        // mirror
        void setMirrorEnabled(bool enabled);
        bool mirrorEnabled() const;

        // misc
        void ensureVisible(const QRectF & viewportRect);
        bool beingTransformed() const;

        // to be reimplemented by subclasses
        virtual QString contentName() const = 0;
        virtual QWidget * createPropertyWidget();
        virtual bool fromXml(QDomElement & parentElement);
        virtual void toXml(QDomElement & parentElement) const;
        virtual QPixmap renderContent(const QSize & size, Qt::AspectRatioMode ratio) const = 0;

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

        // may be reimplemented by subclasses
        virtual int contentHeightForWidth(int width) const;
        virtual bool contentOpaque() const;

    Q_SIGNALS:
        void configureMe(const QPoint & scenePoint);
        void changeStack(int opcode);
        void backgroundMe();
        void deleteItem();
        void contentChanged();

    protected:
        // may be reimplemented by subclasses
        virtual void selectionChanged(bool selected);

        // called by subclasses too
        void GFX_CHANGED() const;
        void setControlsVisible(bool visible);
        QPixmap ratioScaledPixmap(const QPixmap * source, const QSize & size, Qt::AspectRatioMode ratio) const;

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
        void applyRotations();
        QRect               m_contentsRect;
        QRectF              m_frameRect;
        Frame *             m_frame;
        QGraphicsTextItem * m_frameTextItem;
        QList<ButtonItem *> m_controlItems;
        QList<CornerItem *> m_cornerItems;
        bool                m_controlsVisible;
        bool                m_dirtyTransforming;
        QTimer *            m_transformRefreshTimer;
        QTimer *            m_gfxChangeTimer;
        MirrorItem *        m_mirrorItem;
        double              m_xRotationAngle;
        double              m_yRotationAngle;
        double              m_zRotationAngle;

    private Q_SLOTS:
        void slotPerspective(const QPointF & sceneRelPoint, Qt::KeyboardModifiers modifiers);
        void slotClearPerspective();
        void slotDirtyEnded();

        // used by desk arrangement functions
    public:
        Vector2 vForce, vVel, vPos;
};

#endif
