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
class AbstractProperties;
class ButtonItem;
class Frame;
class MirrorItem;
class QGraphicsTextItem;
class QPointF;
class XmlRead;
class XmlSave;


/// \brief Base class of Canvas Item (with lots of gadgets!)
class AbstractContent : public QObject, public QGraphicsItem
{
    Q_OBJECT
    public:
        friend class XmlRead;
        friend class XmlSave;
        AbstractContent(QGraphicsScene * scene, QGraphicsItem * parent = 0, bool noResize = false);
        virtual ~AbstractContent();

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

        void setSelected(bool);

        void setRotation(int, int, int);

        // misc
        void resize(const QRectF & rect);
        void adjustSize();
        void ensureVisible(const QRectF & viewportRect);
        bool beingTransformed() const;

        virtual QPixmap renderAsBackground(const QSize & size) const;
        virtual AbstractProperties * createProperties() const;

        // ::QGraphicsItem
        QRectF boundingRect() const;
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    Q_SIGNALS:
        void gfxChange();
        void configureMe(const QPoint & scenePoint);
        void changeStack(int);
        void backgroundMe();
        void deleteItem();
        void itemSelected(AbstractContent *);
        void addItemToSelection(AbstractContent *);

    protected:
        // useful to sunclasses
        QRect contentsRect() const;
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
        void layoutChildren();
        void applyTransformations();
        QRectF              m_rect;
        Frame *             m_frame;
        QGraphicsTextItem * m_frameTextItem;
        QList<ButtonItem *> m_controlItems;
        float               m_scaleRatio;
        bool                m_transforming;
        bool                m_isSelected;
        QTimer *            m_transformRefreshTimer;
        QTimer *            m_gfxChangeTimer;
        MirrorItem *        m_mirrorItem;
        int m_xRotationAngle, m_yRotationAngle, m_zRotationAngle;

    private Q_SLOTS:
        void slotScaleStarted();
        void slotScale(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers);
        void slotRotate(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers);
        void slotTransformXY(const QPointF&,Qt::KeyboardModifiers);
        void slotResetRatio();
        void slotResetRotation();
        void slotTransformEnded();
};

#endif
