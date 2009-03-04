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
#include <QPainter>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QKeyEvent>
#include <math.h>

// from FotoWall.cpp
extern bool globalExportingFlag;

#define SCALE_X     (- 29 - 5)
#define SCALE_Y     (- 8 -5)
#define ROTATE_X    (- 8 - 5)
#define ROTATE_Y    (- 8 -5)
#define FW_MARGIN   10
#define FW_LABH     30

class MyTextItem : public QGraphicsTextItem {
    public:
        MyTextItem(QGraphicsItem * parent = 0)
            : QGraphicsTextItem(parent)
        {
        }

        void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 ) {
            qWarning("Icepted");
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
    connect(m_scaleButton, SIGNAL(reset()), this, SLOT(slotResetSize()));
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
        slotResetSize();
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
    m_frame->paint(painter, boundingRect().toRect());

    if (!m_photo)
        return;

    // draw high-resolution photo when exporting png
    QRect targetRect(-m_size.width()/2 + FW_MARGIN, -m_size.height()/2 + FW_MARGIN, m_size.width() - 2*FW_MARGIN, m_size.height() - 2*FW_MARGIN - FW_LABH);
    if (globalExportingFlag) {
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
    m_scaleButton->setPos(m_size.width() / 2.0 + SCALE_X, m_size.height() / 2.0 + SCALE_Y);
    m_rotateButton->setPos(m_size.width() / 2.0 + ROTATE_X, m_size.height() / 2.0 + ROTATE_Y);
    m_textItem->setPos(-m_size.width() / 2.0 + FW_MARGIN, m_size.height() / 2.0 - FW_MARGIN - FW_LABH + FW_MARGIN);
}

void FWFoto::slotResize(const QPointF & controlPoint)
{
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = m_rotateButton->pos();
    if (newPos == refPos)
        return;

    // determine the new size
    QSize newSize = QSize((newPos.x() - SCALE_X) * 2.0, (newPos.y() - SCALE_Y) * 2.0);
    if (newSize.width() < (2*FW_MARGIN + 40))
        newSize.setWidth(2*FW_MARGIN + 40);
    if (newSize.height() < (2*FW_MARGIN + 30 + FW_LABH))
        newSize.setHeight(2*FW_MARGIN + 30 + FW_LABH);
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

void FWFoto::slotResetSize()
{
    int width = m_size.width();
    int height = m_size.height();
    if (m_photo) {
        qreal ratio = (qreal)m_photo->width() / (qreal)m_photo->height();
        if (ratio != 0.0) {
            int hfw = (int)(((qreal)width - 20.0) / ratio);
            height = FW_MARGIN + hfw + FW_LABH + FW_MARGIN;
        }
    }
    prepareGeometryChange();
    m_size = QSize(width, height);
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
    if (option->state & QStyle::State_MouseOver) {
        painter->setBrush(m_brush);
        painter->setPen(Qt::white);
        painter->setOpacity(0.8);
        painter->drawEllipse(boundingRect());
        painter->setOpacity(1.0);
    }
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

