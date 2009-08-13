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

#include "GroupBoxWidget.h"
#include <QFontMetrics>
#include <QPainter>
#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

GroupBoxWidget::GroupBoxWidget(QWidget * parent)
  : QWidget(parent)
  , m_shading(0.0)
{
    m_titleFont = font();
    m_titleFont.setPixelSize(10);
}

QString GroupBoxWidget::title() const
{
    return m_titleText;
}

void GroupBoxWidget::setTitle(const QString & title)
{
    m_titleText = title;
    setContentsMargins(0, m_titleText.isEmpty() ? 0 : m_titleFont.pixelSize() + 2, 0, 0);
    update();
}

int GroupBoxWidget::titleSize() const
{
    return m_titleFont.pixelSize();
}

void GroupBoxWidget::setTitleSize(int titleSize)
{
    m_titleFont.setPointSize(titleSize);
    setContentsMargins(0, m_titleText.isEmpty() ? 0 : m_titleFont.pixelSize() + 2, 0, 0);
    update();
}

qreal GroupBoxWidget::shading() const
{
    return m_shading;
}

void GroupBoxWidget::setShading(qreal value)
{
    m_shading = value;
    update();
}

void GroupBoxWidget::collapse()
{
#if QT_VERSION >= 0x040600
    QPropertyAnimation * ani = new QPropertyAnimation(this, "fixedWidth", this);
    ani->setEasingCurve(QEasingCurve::OutQuint);
    ani->setDuration(200);
    ani->setEndValue(0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    setMinimumWidth(0);
    hide();
#endif
}

void GroupBoxWidget::expand()
{
#if QT_VERSION >= 0x040600
    QPropertyAnimation * ani = new QPropertyAnimation(this, "fixedWidth", this);
    ani->setEasingCurve(QEasingCurve::OutQuint);
    ani->setDuration(300);
    ani->setEndValue(calcMinWidth());
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    setMinimumWidth(calcMinWidth());
    show();
#endif
}

void GroupBoxWidget::enterEvent(QEvent *)
{
#if QT_VERSION >= 0x040600
    QPropertyAnimation * ani = new QPropertyAnimation(this, "shading", this);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(400);
    ani->setEndValue(1.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    setShading(1.0);
#endif
}

void GroupBoxWidget::leaveEvent(QEvent *)
{
#if QT_VERSION >= 0x040600
    QPropertyAnimation * ani = new QPropertyAnimation(this, "shading", this);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(400);
    ani->setEndValue(0.0);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
#else
    setShading(0.0);
#endif
}

void GroupBoxWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter p(this);
    if (m_shading > 0) {
        QRadialGradient rg(0.5, 0.2, 0.8);
        QColor startColor(Qt::white);
        startColor.setAlpha((int)(255.0 * m_shading));
        rg.setColorAt(0.0, startColor);
        rg.setColorAt(1.0, Qt::transparent);
        rg.setCoordinateMode(QGradient::ObjectBoundingMode);
        p.fillRect(0, 0, width(), 12, rg);
    }

    if (!m_titleText.isEmpty()) {
        p.setFont(m_titleFont);
        p.drawText(QRect(0, 0, width(), m_titleFont.pixelSize() + 4), Qt::AlignHCenter | Qt::AlignTop, m_titleText);
    }
}

int GroupBoxWidget::calcMinWidth()
{
    QFontMetrics metrics(m_titleFont);
    return qMax(metrics.width(m_titleText) + 12, QWidget::sizeHint().width());
}
