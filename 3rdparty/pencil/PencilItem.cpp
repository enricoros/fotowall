/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "PencilItem.h"

#include "FloodGenerator.h"
#include "FloodPoly.h"

#include <QApplication>
#include <QGraphicsPathItem>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPropertyAnimation>

#if QT_VERSION >= 0x040600

PencilItem::PencilItem(const QString & svgFileName, QGraphicsItem *parent)
  : QGraphicsObject(parent)
  , m_alpha(0)
{
    FloodPolys polys = FloodGenerator::fromSvgPaths(svgFileName);
    if (polys.isEmpty())
        return;

    // generate the steps from the loaded FloodPolys
    QPainterPath completePath;
    foreach (const FloodPoly & poly, polys) {
        Step step;
        step.move = true;
        const QPainterPath path = poly.toPainterPath();
        completePath.addPath(path);
        qreal length = path.length();
        int breaks = (int)(length / 2.0);
        for (int i = 0; i < breaks; i++) {
            step.point = path.pointAtPercent((qreal)i / ((qreal)breaks - 1));
            m_steps.append(step);
            step.move = false;
        }
    }

    // set rect to the boundary of the path
    m_rect = completePath.boundingRect();
    if (!m_rect.isValid())
        return;

    // start the animation
    QPropertyAnimation * ani = new QPropertyAnimation(this, "alpha", this);
    ani->setEndValue(1.0);
    ani->setEasingCurve(QEasingCurve::Linear);
    ani->setDuration(5000);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF PencilItem::boundingRect() const
{
    return m_rect;
}

void PencilItem::paint(QPainter *painter, const QStyleOptionGraphicsItem */*option*/, QWidget */*widget*/)
{
    int end = (int)((qreal)m_steps.size() * 1.4 * m_alpha);
    int width = (int)((qreal)m_steps.size() * 0.4);
    int start = end - width;
    int stop = qMin(m_steps.size(), end);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 1));
    QPointF p1;
    for (int idx = qMax(0, start); idx < stop; ++idx) {
        const Step & step = m_steps.at(idx);
        if (step.move || p1.isNull())
            p1 = step.point;
        else {
            qreal opacity = (qreal)(idx - start) / (qreal)width;
            painter->setOpacity(opacity);
            painter->drawLine(p1, step.point);
            p1 = step.point;
        }
    }
}

qreal PencilItem::alpha() const
{
    return m_alpha;
}

void PencilItem::setAlpha(qreal alpha)
{
    if (m_alpha != alpha) {
        m_alpha = alpha;
        update();
    }
}

#endif
