/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CornerItem.h"
#include "AbstractContent.h"
#include "RenderOpts.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <math.h>

CornerItem::CornerItem(Qt::Corner corner, AbstractContent * parent)
    : QGraphicsItem(parent)
    , m_content(parent)
    , m_corner(corner)
    , m_opMask(0xFFFF)
    , m_side(8)
    , m_operation(Off)
{
    setAcceptsHoverEvents(true);
    //setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

void CornerItem::relayout(const QRect & rect)
{
    // change side, if needed
    int side = 1 + sqrt(qMin(rect.width(), rect.height()));
    if (side != m_side) {
        prepareGeometryChange();
        m_side = side;
    }

    // place at the right corner
    switch (m_corner) {
        case Qt::TopLeftCorner: setPos(rect.topLeft() + QPoint(m_side, m_side)); break;
        case Qt::TopRightCorner: setPos(rect.topRight() + QPoint(-m_side + 1, m_side)); break;
        case Qt::BottomLeftCorner: setPos(rect.bottomLeft() + QPoint(m_side, -m_side + 1)); break;
        case Qt::BottomRightCorner: setPos(rect.bottomRight() + QPoint(-m_side + 1, -m_side + 1)); break;
    }
}

QRectF CornerItem::boundingRect() const
{
    return QRectF(-m_side, -m_side, 2*m_side, 2*m_side);
}

void CornerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();

    // select the right op
    switch (event->button()) {
        case Qt::LeftButton:    m_content->resetContentsRatio(); break;
        case Qt::RightButton:   m_content->setTransform(QTransform(), false); break;
        default:                break;
    }
}

void CornerItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();

    // select the right op
    switch (event->button()) {
        case Qt::LeftButton:    m_operation = Scale | FixScale; break;
        case Qt::RightButton:   m_operation = Rotate | Scale | FixScale; break;
        case Qt::MidButton:     m_operation = Scale; break;
        default:                m_operation = Off; return;
    }

    // filter out unwanted operations
    m_operation &= m_opMask;

    // intial parameters
    QRect contentsRect = m_content->contentsRect();
    m_startRatio = (double)contentsRect.width() / (double)contentsRect.height();

    update();
}

void CornerItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (m_operation == Off)
        return;
    event->accept();

    // modify the operation using the shortcuts
    int op = m_operation;
    if (event->modifiers() & Qt::ShiftModifier)
        op &= ~FixScale;
    if (event->modifiers() & Qt::ControlModifier)
        op |= FixRotate;
    if (event->modifiers() & Qt::AltModifier)
        op &= ~(Scale | FixScale);
    op &= m_opMask;
    if ((op & (Rotate | Scale)) == (Rotate | Scale))
        op |= FixScale;
    if (op == Off)
        return;

    // vector relative to the centre
    QPointF v = pos() + event->pos();
    if (v.isNull())
        return;

    // do scaling
    m_content->delayedDirty();
    if (op & Scale) {
        if (op & FixScale) {
            const double r = m_startRatio;
            const double D = sqrt(v.x()*v.x() + v.y()*v.y());
            const double K = sqrt(1 + 1/(r * r));
            int W = (int)((2*D)/(K));
            int H = (int)((2*D)/(r*K));
            m_content->resizeContents(QRect(-W / 2, -H / 2, W, H));
        } else {
            int W = qMax(2 * v.x(), 50.0); //(m_contentsRect.width() * v.x()) / oldPos.x();
            int H = qMax(2 * v.y(), 40.0); //(m_contentsRect.height() * v.y()) / oldPos.y();
            //if (W != (int)cRect.width() || H != (int)cRect.height())
                m_content->resizeContents(QRect(-W / 2, -H / 2, W, H));
        }
    }

    // do rotation
    if (op & Rotate) {
        QPointF refPos = pos();

        // set item rotation (set rotation relative to current)
        qreal refAngle = atan2(refPos.y(), refPos.x());
        qreal newAngle = atan2(v.y(), v.x());
        //m_zRotationAngle += 57.29577951308232 * newAngle; // 180 * a / M_PI
        //QTransform zTransform = QTransform().rotate(57.29577951308232 * (newAngle - refAngle), Qt::ZAxis); // 180 * a / M_PI
        //setTransform(zTransform, true);
        m_content->rotate(57.29577951308232 * (newAngle - refAngle));

        // snap to M_PI/4
        if (op & FixRotate) {
            QTransform t = m_content->transform();
            QPointF ax = t.map(QPointF(1, 0));
            qreal rotAngle = atan2(ax.y(), ax.x());
            int fracts = (int)((rotAngle - (0.19635/2)) / (0.39270/2));
            rotAngle = (qreal)fracts * (0.39270/2);
            qWarning() << rotAngle << fracts;
            m_content->setTransform(QTransform().rotateRadians(rotAngle));
        }
        //m_content->setTransform(QTransform().rotate(m_zRotationAngle, Qt::ZAxis).rotate(m_yRotationAngle, Qt::YAxis).rotate(m_xRotationAngle, Qt::XAxis));
    }
}

void CornerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    event->accept();
    bool accepted = m_operation != Off;
    m_operation = Off;
    update();

    // clicked
    if (accepted) {
    }
}

void CornerItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
    if (RenderOpts::HQRendering)
        return;

    QColor color = RenderOpts::hiColor;
    if (option->state & QStyle::State_MouseOver) {
        if (m_operation != Off)
            color.setAlpha(250);
        else
            color.setAlpha(128);
    } else
        color.setAlpha(64);
    painter->fillRect(boundingRect(), color);
}
