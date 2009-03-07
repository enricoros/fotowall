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

#include "PictureItem.h"
#include "ButtonItem.h"
#include "frames/Frame.h"
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <QUrl>
#include <math.h>

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

PictureItem::PictureItem(QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_frame(0)
    , m_photo(0)
    , m_size(200, 150)
    , m_scaleRefreshTimer(0)
    , m_scaling(false)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    // create child items
    m_scaleButton = new ButtonItem(this, Qt::green, QIcon(":/data/action-scale.png"));
    m_scaleButton->hide();
    connect(m_scaleButton, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotResize(const QPointF&,Qt::KeyboardModifiers)));
    connect(m_scaleButton, SIGNAL(doubleClicked()), this, SLOT(slotResetAspectRatio()));

    m_rotateButton = new ButtonItem(this, Qt::green, QIcon(":/data/action-rotate.png"));
    m_rotateButton->hide();
    connect(m_rotateButton, SIGNAL(dragging(const QPointF&,Qt::KeyboardModifiers)), this, SLOT(slotRotate(const QPointF&)));
    connect(m_rotateButton, SIGNAL(doubleClicked()), this, SLOT(slotResetRotation()));

    m_frontButton = new ButtonItem(this, Qt::blue, QIcon(":/data/action-order-front.png"));
    m_frontButton->hide();
    connect(m_frontButton, SIGNAL(clicked()), this, SIGNAL(raiseMe()));

    m_deleteButton = new ButtonItem(this, Qt::red, QIcon(":/data/action-delete.png"));
    m_deleteButton->hide();
    connect(m_deleteButton, SIGNAL(clicked()), this, SIGNAL(deleteMe()));

    m_textItem = new MyTextItem(this);
    m_textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont f("Serif");
    f.setPointSizeF(7.5);
    m_textItem->setFont(f);
    m_textItem->setPlainText(tr("..."));

    relayoutContents();
}

PictureItem::~PictureItem()
{
    delete m_frame;
}

bool PictureItem::loadPhoto(const QString & fileName, bool keepRatio, bool setName)
{
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_photo = new QPixmap(fileName);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        m_fileName = QString();
        return false;
    }
    m_fileName = fileName;
    if (keepRatio)
        slotResetAspectRatio();
    if (setName)
        m_textItem->setPlainText(QFileInfo(fileName).fileName().section('.', 0, 0) + QString("..."));
    update();
    return true;
}

QPixmap PictureItem::renderPhoto(const QSize & size) const
{
    if (m_photo)
        return m_photo->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return QPixmap();
}

void PictureItem::setFrame(Frame * frame)
{
    delete m_frame;
    m_frame = frame;
    slotResetAspectRatio();
    relayoutContents();
    update();
}

void PictureItem::save(QDataStream & data) const
{
    data << m_size;
    data << pos();
    data << transform();
    data << zValue();
    data << m_fileName;
    data << m_textItem->toPlainText();
}

bool PictureItem::restore(QDataStream & data)
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
    bool ok = loadPhoto(fileName);
    QString text;
    data >> text;
    m_textItem->setPlainText(text);
    update();
    return ok;
}

void PictureItem::ensureVisible(const QRectF & rect)
{
    // keep the center inside the scene rect
    QPointF center = pos();
    if (!rect.contains(center)) {
        center.setX(qBound(rect.left(), center.x(), rect.right()));
        center.setY(qBound(rect.top(), center.y(), rect.bottom()));
        setPos(center);
    }
}

QRectF PictureItem::boundingRect() const
{
    return QRectF(-m_size.width()/2, -m_size.height()/2, m_size.width(), m_size.height());
}

void PictureItem::hoverEnterEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    m_scaleButton->show();
    m_rotateButton->show();
    m_frontButton->show();
    m_deleteButton->show();
}

void PictureItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * /*event*/)
{
    m_scaleButton->hide();
    m_rotateButton->hide();
    m_frontButton->hide();
    m_deleteButton->hide();
}

void PictureItem::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void PictureItem::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // load the first valid picture
    foreach (QUrl url, event->mimeData()->urls()) {
        if (loadPhoto(url.toLocalFile(), true, false)) {
            event->accept();
            return;
        }
    }
}

void PictureItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // draw the dia background
    if (!m_frame)
        return;
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

void PictureItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_frame)
        if (m_frame->contentsRect(boundingRect().toRect()).contains(event->pos().toPoint()))
            emit backgroundMe();
}

void PictureItem::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    int newZValue = (int)(zValue() - event->delta() / 120);
    setZValue(newZValue);
}

void PictureItem::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Delete)
        emit deleteMe();
    event->accept();
}

QVariant PictureItem::itemChange(GraphicsItemChange change, const QVariant & value)
{
    // keep the center inside the scene rect..
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        QRectF rect = scene()->sceneRect();
        if (!rect.contains(newPos)) {
            newPos.setX(qBound(rect.left(), newPos.x(), rect.right()));
            newPos.setY(qBound(rect.top(), newPos.y(), rect.bottom()));
            return newPos;
        }
    }
    // ..or just apply the value
    return QGraphicsItem::itemChange(change, value);
}

void PictureItem::relayoutContents()
{
    if (!m_frame)
        return;

    // layout buttons and text
    QList<QGraphicsItem *> buttons;
    buttons << m_scaleButton << m_rotateButton << m_frontButton << m_deleteButton;
    m_frame->layoutButtons(buttons, boundingRect().toRect());
    m_frame->layoutText(m_textItem, boundingRect().toRect());
}

void PictureItem::slotResize(const QPointF & controlPoint, Qt::KeyboardModifiers modifiers)
{
    QPoint newPos = mapFromScene(controlPoint).toPoint();
    QPoint oldPos = m_scaleButton->pos().toPoint();
    if (newPos == oldPos)
        return;

    // determine the new size
    QSize newSize((m_size.width() * newPos.x()) / oldPos.x(), (m_size.height() * newPos.y()) / oldPos.y());
    if (modifiers != Qt::NoModifier && m_photo)
        newSize.setHeight((m_photo->height() * m_size.width()) / m_photo->width());
    if (newSize.width() < 160)
        newSize.setWidth(160);
    if (newSize.height() < 90)
        newSize.setHeight(90);
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

void PictureItem::slotRotate(const QPointF & controlPoint)
{
    QPointF newPos = mapFromScene(controlPoint);
    QPointF refPos = m_rotateButton->pos();
    if (newPos == refPos)
        return;

    // set item rotation (set rotation relative to current)
    qreal refAngle = atan2(refPos.y(), refPos.x());
    qreal newAngle = atan2(newPos.y(), newPos.x());
    rotate(57.29577951308232 * (newAngle - refAngle)); // 180 * a / M_PI
}

void PictureItem::slotResetAspectRatio()
{
    // get the new size
    if (!m_photo || m_photo->isNull() || !m_frame)
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

void PictureItem::slotResetRotation()
{
    QTransform ident;
    setTransform(ident, false);
}

void PictureItem::slotResizeEnded()
{
    m_scaling = false;
    update();
}
