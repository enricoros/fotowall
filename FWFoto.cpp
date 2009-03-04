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

#include "FWFoto.h"
#include "frames/Frame.h"
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTimer>
#include <QUrl>
#include <math.h>

#define notImplemented() {qWarning("%s:%d: %s NOT Implemented!", __FILE__, __LINE__, __FUNCTION__);}

// from FotoWall.cpp
extern bool globalExportingFlag;

class MyTextItem : public QGraphicsTextItem {
    public:
        MyTextItem(QGraphicsItem * parent = 0)
            : QGraphicsTextItem(parent)
        {
        }

        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
            painter->save();
            painter->setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform );
            QGraphicsTextItem::paint(painter, option, widget);
            painter->restore();
        }
};

FWFoto::FWFoto( Frame * frame, QGraphicsItem * parent )
    : QGraphicsItem(parent)
    , m_frame( frame )
    , m_photo(0)
    , m_size(200, 150)
    , m_scaleRefreshTimer(0)
    , m_scaling(false)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);

    // create child items
    m_scaleButton = new FWButton(this, Qt::red);
    connect(m_scaleButton, SIGNAL(dragging(const QPointF&)), this, SLOT(slotResize(const QPointF&)));
    connect(m_scaleButton, SIGNAL(reset()), this, SLOT(slotResetAspectRatio()));

    m_rotateButton = new FWButton(this, Qt::green);
    connect(m_rotateButton, SIGNAL(dragging(const QPointF&)), this, SLOT(slotRotate(const QPointF&)));
    connect(m_rotateButton, SIGNAL(reset()), this, SLOT(slotResetRotation()));

    m_textItem = new MyTextItem(this);
    m_textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont f("Serif");
    f.setPointSizeF(7.5);
    m_textItem->setFont(f);
    m_textItem->setPlainText(tr("..."));

    relayoutContents();
}

FWFoto::~FWFoto()
{
    delete m_frame;
}

void FWFoto::loadPhoto(const QString & fileName, bool keepRatio, bool setName)
{
    delete m_photo;
    m_photo = new QPixmap(fileName);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        m_fileName = QString();
    }
    m_fileName = fileName;
    if (keepRatio)
        slotResetAspectRatio();
    if (setName)
        m_textItem->setPlainText(QFileInfo(fileName).fileName().section('.', 0, 0) + QString("..."));
    update();
}

void FWFoto::save(QDataStream & data) const
{
    data << m_size;
    data << pos();
    data << transform();
    data << zValue();
    data << m_fileName;
    data << m_textItem->toPlainText();
}

void FWFoto::restore(QDataStream & data)
{
    prepareGeometryChange();
    data >> m_size;
    relayoutContents();
    QPointF p;
    data >> p;
    setPos(p);
    QTransform t;
    data >> t;
    setTransform(t);
    qreal zVal;
    data >> zVal;
    setZValue(zVal);
    QString fileName;
    data >> fileName;
    loadPhoto(fileName);
    QString text;
    data >> text;
    m_textItem->setPlainText(text);
    update();
}

QRectF FWFoto::boundingRect() const
{
    return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
}

void FWFoto::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // draw the dia background
    QRect frameRect = boundingRect().toRect();
    m_frame->paint(painter, frameRect);
    if (!m_photo)
        return;
    if (m_frame->clipContents())
        painter->setClipPath(m_frame->contentsClipPath(frameRect));
    QRect targetRect = m_frame->contentsRect(frameRect);

    // draw high-resolution photo when exporting png
    if (globalExportingFlag) {
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect, *m_photo);
        return;
    }

    // draw photo using caching and deferred rescales
    if (m_scaling) {
        if (!m_cachedPhoto.isNull())
            painter->drawPixmap(targetRect, m_cachedPhoto);
    } else {
        if (m_cachedPhoto.isNull() || m_cachedPhoto.size() != targetRect.size())
            m_cachedPhoto = m_photo->scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect.topLeft(), m_cachedPhoto);
    }
}

void FWFoto::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    int newZValue = (int)(zValue() - event->delta() / 120);
    setZValue(newZValue);
}

void FWFoto::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Delete)
        emit deletePressed();
    event->accept();
}

void FWFoto::relayoutContents()
{
    QList<QGraphicsItem *> buttons;
    buttons.append(m_scaleButton);
    buttons.append(m_rotateButton);
    m_frame->layoutButtons(buttons, boundingRect().toRect());
    m_frame->layoutText(m_textItem, boundingRect().toRect());
}

void FWFoto::slotResize(const QPointF & controlPoint)
{
    QPoint newPos = mapFromScene(controlPoint).toPoint();
    QPoint oldPos = m_scaleButton->pos().toPoint();
    if (newPos == oldPos)
        return;

    // determine the new size
    QSize newSize((m_size.width() * newPos.x()) / oldPos.x(), (m_size.height() * newPos.y()) / oldPos.y());
    if (newSize.width() < 100)
        newSize.setWidth(100);
    if (newSize.height() < 100)
        newSize.setHeight(100);
    if (newSize == m_size)
        return;

    // change geometry
    m_scaling = true;
    prepareGeometryChange();
    m_size = newSize;
    relayoutContents();
    update();

    // start refresh timer
    if (!m_scaleRefreshTimer) {
        m_scaleRefreshTimer = new QTimer(this);
        connect(m_scaleRefreshTimer, SIGNAL(timeout()), this, SLOT(slotResizeEnded()));
        m_scaleRefreshTimer->setSingleShot(true);
    }
    m_scaleRefreshTimer->start(400);
}

void FWFoto::slotRotate(const QPointF & controlPoint)
{
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = m_rotateButton->pos();
    if (newPos == refPos)
        return;

    // set item rotation (set rotation relative to current)
    qreal refAngle = atan2(refPos.y(), refPos.x());
    qreal newAngle = atan2(newPos.y(), newPos.x());
    rotate(180.0 * (newAngle - refAngle) / M_PI);
}

void FWFoto::slotResetAspectRatio()
{
    // get the new size
    if (!m_photo || m_photo->isNull())
        return;
    QSize newSize = m_frame->sizeForContentsRatio(m_size.width(), (qreal)m_photo->width() / (qreal)m_photo->height());
    if (newSize == m_size)
        return;

    // apply the new size
    prepareGeometryChange();
    m_size = newSize;
    relayoutContents();
    update();
}

void FWFoto::slotResetRotation()
{
    QTransform ident;
    setTransform(ident, false);
}

void FWFoto::slotResizeEnded()
{
    m_scaling = false;
    update();
}




FWButton::FWButton(FWFoto * parent, const QBrush & brush)
    : QGraphicsItem(parent)
    , m_brush(brush)
{
    setAcceptsHoverEvents(true);
}

QRectF FWButton::boundingRect() const
{
    return QRectF(-8, -8, 16, 16);
}

void FWButton::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
    if (globalExportingFlag)
        return;
    painter->setBrush(m_brush);
    painter->setPen(Qt::white);
    painter->setOpacity((option->state & QStyle::State_MouseOver) ? 0.8 : 0.2);
    painter->drawEllipse(boundingRect());
    painter->setOpacity(1.0);
}

void FWButton::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    m_startPos = event->scenePos();
}

void FWButton::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_startPos.isNull())
        return;
    event->accept();
    emit dragging(event->scenePos());
}

void FWButton::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    m_startPos = QPointF();
}

void FWButton::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    emit reset();
}

