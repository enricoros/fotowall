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
#include <QVariant>
#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#define ANIMATE_PARAM(propName, duration, endValue, fixupLayout) \
    {QPropertyAnimation * ani = new QPropertyAnimation(this, propName, this); \
    if (fixupLayout) connect(ani, SIGNAL(finished()), this, SLOT(slotFixupLayout())); \
    ani->setEasingCurve(QEasingCurve::OutQuint); \
    ani->setDuration(duration); \
    ani->setEndValue(endValue); \
    ani->start(QPropertyAnimation::DeleteWhenStopped);}
#else
#define ANIMATE_PARAM(propName, duration, endValue, fixupLayout) \
    setProperty(propName, endValue); \
    if (fixupLayout) slotFixupLayout();
#endif

GroupBoxWidget::GroupBoxWidget(QWidget * parent)
  : QWidget(parent)
  , m_collapsed(false)
  , m_checkable(false)
  , m_checked(true)
  , m_checkValue(1.0)
  , m_hoverValue(0.0)
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
    recalcLayout();
}

int GroupBoxWidget::titleSize() const
{
    return m_titleFont.pixelSize();
}

void GroupBoxWidget::setTitleSize(int titleSize)
{
    m_titleFont.setPointSize(titleSize);
    recalcLayout();
}

bool GroupBoxWidget::isCheckable() const
{
    return m_checkable;
}

void GroupBoxWidget::setCheckable(bool checkable)
{
    if (m_checkable == checkable)
        return;
    m_checkable = checkable;
    recalcLayout();
}

bool GroupBoxWidget::isChecked() const
{
    return m_checkable ? m_checked : false;
}

void GroupBoxWidget::setChecked(bool checked)
{
    if (!m_checkable || m_checked == checked)
        return;
    m_checked = checked;
    emit toggled(checked);
    recalcLayout();
}

void GroupBoxWidget::collapse()
{
    m_collapsed = true;
    recalcLayout();
}

void GroupBoxWidget::expand()
{
    m_collapsed = false;
    recalcLayout();
}

void GroupBoxWidget::enterEvent(QEvent *)
{
    ANIMATE_PARAM("hAnim", 400, 1.0, false);
}

void GroupBoxWidget::leaveEvent(QEvent *)
{
    ANIMATE_PARAM("hAnim", 400, 0.0, false);
}

void GroupBoxWidget::mousePressEvent(QMouseEvent * /*event*/)
{
    setChecked(!isChecked());
}

void GroupBoxWidget::paintEvent(QPaintEvent * /*event*/)
{
    // skip painting if no text
    if (m_titleText.isEmpty())
        return;

    // draw hovering
    QPainter p(this);
    if (m_hoverValue > 0 && (m_checkValue == 0.0 || m_checkValue == 1.0)) {
        QRadialGradient rg = m_checkValue == 1.0 ? QRadialGradient(0.5, 0.2, 0.8) : QRadialGradient(0.5, 0.9, 1.5);
        QColor startColor(Qt::white);
        startColor.setAlpha((int)(255.0 * m_hoverValue));
        rg.setColorAt(0.0, startColor);
        rg.setColorAt(1.0, Qt::transparent);
        rg.setColorAt(0.0, startColor);
        rg.setColorAt(1.0, Qt::transparent);
        rg.setCoordinateMode(QGradient::ObjectBoundingMode);
        p.fillRect(0, 0, width(), height() - m_checkValue * (height() - 12) , rg);
    }

    // draw text
    p.setFont(m_titleFont);
    QFontMetrics metrics(m_titleFont);
    QRect textRect = metrics.boundingRect(m_titleText);
    if (m_checkValue < 1.0) {
        qreal xStart = -textRect.top() + 2,
              xStop = (width() - textRect.width()) / 2.0;
        qreal yStart = height() - 2, //height() + textRect.width()) / 2.0,
              yStop = -textRect.top();
        qreal xT = xStart + m_checkValue * (xStop - xStart);
        qreal yT = yStart + m_checkValue * (yStop - yStart);

        p.save();
        p.translate(xT, yT);
        p.rotate(m_checkValue * 90 - 90);
        p.drawText(0, 0, m_titleText);
        p.restore();
    } else
        p.drawText((width() - textRect.width()) / 2.0, -textRect.top(), m_titleText);
}


qreal GroupBoxWidget::checkValue() const
{
    return m_checkValue;
}

void GroupBoxWidget::setCheckValue(qreal value)
{
    m_checkValue = value;
    update();
}

qreal GroupBoxWidget::hoverValue() const
{
    return m_hoverValue;
}

void GroupBoxWidget::setHoverValue(qreal value)
{
    m_hoverValue = value;
    update();
}

void GroupBoxWidget::recalcLayout()
{
    qWarning("RL");
    // full collapse: shrink to zero
    if (m_collapsed) {
        ANIMATE_PARAM("fixedWidth", 200, 0, true)
        return;
    }

    // checkable off: shrink to the target width
    int textHeight = m_titleText.isEmpty() ? 0 : m_titleFont.pixelSize() + 8;
    if (m_checkable && !m_checked) {
        setContentsMargins(textHeight, 0, 0, 0);
        setCursor(Qt::PointingHandCursor);
        ANIMATE_PARAM("fixedWidth", 200, textHeight, true);
        ANIMATE_PARAM("cAnim", 300, 0.0, false);
        return;
    }

    // normal: expand to the full size
    setContentsMargins(0, textHeight, 0, 0);
    setCursor(Qt::ArrowCursor);
    QFontMetrics metrics(m_titleFont);
    int minWidth = qMax(metrics.width(m_titleText) + 12, QWidget::sizeHint().width());
    ANIMATE_PARAM("fixedWidth", 300, minWidth, true);
    ANIMATE_PARAM("cAnim", 400, 1.0, false);
}

void GroupBoxWidget::slotFixupLayout()
{
    if (m_collapsed) {
        setFixedWidth(0);
    } else if (m_checkable && !m_checked) {
        setFixedWidth(m_titleFont.pixelSize() + 8);
        setCheckValue(0.0);
    } else  {
        setMaximumWidth(QWIDGETSIZE_MAX);
        setCheckValue(1.0);
    }
}
