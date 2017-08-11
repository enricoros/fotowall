/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
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

#include "Shared/AbstractDisposeable.h"
#include "3rdparty/enricomath.h"
#include <QDomElement>
#include <QDir>
class AbstractConfig;
class ButtonItem;
class ContentProperties;
class CornerItem;
class Frame;
class MirrorItem;
class QGraphicsTextItem;
class QPointF;
class PE_AbstractSlider;

/// \brief Base class of Canvas Item (with lots of gadgets!)
class AbstractContent : public AbstractDisposeable
{
    Q_OBJECT
#if QT_VERSION < 0x040600
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
#endif
    Q_PROPERTY(bool mirrored READ mirrored WRITE setMirrored NOTIFY mirroredChanged)
    Q_PROPERTY(QPointF perspective READ perspective WRITE setPerspective NOTIFY perspectiveChanged)
    Q_PROPERTY(int fxIndex READ fxIndex WRITE setFxIndex NOTIFY fxIndexChanged)
    public:
        AbstractContent(QGraphicsScene * scene, bool fadeIn, bool noRescale, QGraphicsItem * parent);
        virtual ~AbstractContent();

        // ::AbstractDisposeable
        void dispose();

        // size
        QRect contentRect() const;
        void resizeContents(const QRect & rect, bool keepRatio = false);
        void resetContentsRatio();
        void delayedDirty(int ms = 400);

        // position
        QPointF previousPos() const;
        void setPreviousPos(const QPointF& previousPos);

        // frame (and frame text)
        void setFrame(Frame * frame);
        quint32 frameClass() const;
        void setFrameTextEnabled(bool enabled);
        bool frameTextEnabled() const;
        void setFrameTextReadonly(bool);
        bool frameTextReadonly() const;
        void setFrameText(const QString & text);
        QString frameText() const;
        void addButtonItem(ButtonItem * buttonItem);

        // properties
        void setMirrored(bool enabled);
        bool mirrored() const;
        void setPerspective(const QPointF & angles);
        QPointF perspective() const;
#if QT_VERSION < 0x040600
        void setRotation(qreal angle);
        qreal rotation() const;
#endif
        void setFxIndex(int index);
        int fxIndex() const;

        bool locked();
        bool fixedPosition();
        bool fixedRotation();
        bool fixedPerspective();

        // misc
        void ensureVisible(const QRectF & viewportRect);
        bool beingTransformed() const;

        // to be reimplemented by subclasses
        virtual QString contentName() const = 0;
        virtual bool fromXml(QDomElement & contentElement, const QDir & baseDir);
        virtual void toXml(QDomElement & contentElement, const QDir & baseDir) const;
        virtual void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio) = 0;
        virtual QPixmap toPixmap(const QSize & size, Qt::AspectRatioMode ratio);
        virtual int contentHeightForWidth(int width) const;
        virtual bool contentOpaque() const;
        virtual QWidget * createPropertyWidget(ContentProperties * p = 0);

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    Q_SIGNALS:
        // to canvas
        void changeStack(int opcode);
        void requestBackgrounding();
        void requestConfig(const QPoint & scenePoint);
        void requestEditing();
        void requestRemoval();
        void contentChanged();

        // properties
        void mirroredChanged();
        void perspectiveChanged();
#if QT_VERSION < 0x040600
        void rotationChanged();
#endif
        void fxIndexChanged();

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
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
        void keyPressEvent(QKeyEvent * event);
        QVariant itemChange(GraphicsItemChange change, const QVariant & value);

    protected Q_SLOTS:
        void slotConfigure();
        void slotSetLocked(int);
        void slotSetFixedPosition(int);
        void slotSetFixedRotation(int);
        void slotSetFixedPerspective(int);
        void slotStackFront();
        void slotStackRaise();
        void slotStackLower();
        void slotStackBack();
        void slotSaveAs();
        void slotPressPerspective();
        void slotReleasePerspective(QGraphicsSceneMouseEvent *);
        void slotOpacityChanged();
        void slotOpacityChanging();

    private:
        void createCorner(Qt::Corner corner, bool noRescale);
        void layoutChildren();
        void applyTransforms();
        PE_AbstractSlider   *m_opacitySlider;
        QTransform          m_previousTransform;
        QPointF             m_previousPos;
        QRect               m_contentRect;
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
        QPointF             m_perspectiveAngles;
#if QT_VERSION < 0x040600
        double              m_rotationAngle;
#endif
        int                 m_fxIndex;

        bool                m_locked;
        bool                m_fixedPosition;
        bool                m_fixedRotation, m_fixedPerspective;
        qreal               m_opacity;


    private Q_SLOTS:
        void slotSetPerspective(const QPointF & sceneRelPoint, Qt::KeyboardModifiers modifiers);
        void slotClearPerspective();
        void slotDirtyEnded();

        // used by arrangement functions FIXME
    public:
        Vector2 vForce, vVel, vPos;
};


/// \brief Utility function to cast a list
template<typename From, typename To>
QList<To *> projectList(const QList<From *> list)
{
    QList<To *> projectedList;
    foreach (From * item, list)
        if (To * to = dynamic_cast<To *>(item))
            projectedList.append(to);
    return projectedList;
}

#endif
