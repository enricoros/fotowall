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

#include "GroupBoxWidget.h"
#include <QLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionButton>
#include <QStyle>
#include <QTimer>
#include <QVariant>

#if defined(MOBILE_UI) || QT_VERSION < 0x040600
#define ANIMATE_PARAM(propName, duration, endValue, finalizeLayout) \
    {setProperty(propName, endValue); if (finalizeLayout) slotFinalizeDesign();}
#else
#include <QPropertyAnimation>
#define ANIMATE_PARAM(propName, duration, endValue, finalizeLayout) \
    {QPropertyAnimation * ani = new QPropertyAnimation(this, propName, this); \
    if (finalizeLayout) connect(ani, SIGNAL(finished()), this, SLOT(slotFinalizeDesign())); \
    ani->setEasingCurve(QEasingCurve::OutQuint); \
    ani->setDuration(duration); \
    ani->setEndValue(endValue); \
    ani->start(QPropertyAnimation::DeleteWhenStopped);}
#endif

GroupBoxWidget::GroupBoxWidget(QWidget * parent)
  : QWidget(parent)
  , m_redesignTimer(0)
  , m_collapsed(false)
  , m_checkable(false)
  , m_checked(true)
  , m_borderFlags(0)
  , m_checkValue(1.0)
  , m_hoverValue(0.0)
{
    // setup groupbox text
    m_titleFont = font();
    m_titleFont.setPixelSize(10);

    // using a fixed HSizePolicy we better integrate with auto-layouting
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    // autofill off on mobile (inserted on canvas)
#if defined(MOBILE_UI)
    setAutoFillBackground(false);
#else
    setAutoFillBackground(true);
#endif

    // hide junk before initial layouting
    hide();
}

QString GroupBoxWidget::title() const
{
    return m_titleText;
}

void GroupBoxWidget::setTitle(const QString & title)
{
    m_titleText = title;
    updateDesign();
}

int GroupBoxWidget::titleSize() const
{
    return m_titleFont.pixelSize();
}

void GroupBoxWidget::setTitleSize(int titleSize)
{
    m_titleFont.setPointSize(titleSize);
    updateDesign();
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
    updateDesign();
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
    updateDesign();
}

int GroupBoxWidget::borderFlags() const
{
    return m_borderFlags;
}

void GroupBoxWidget::setBorderFlags(int flags)
{
    m_borderFlags = flags;
    update();
}

void GroupBoxWidget::collapse()
{
    if (!m_collapsed) {
        m_collapsed = true;
        updateDesign();
    }
}

void GroupBoxWidget::expand()
{
    if (m_collapsed) {
        m_collapsed = false;
        updateDesign();
    }
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

void GroupBoxWidget::paintEvent(QPaintEvent * event)
{
    // skip the rest of the painting if no text
    if (m_titleText.isEmpty())
        return;

    QPainter p(this);

#if defined(MOBILE_UI)
    // draw light background
    p.fillRect(event->rect(), palette().color(QPalette::Window));
#endif

#if !defined(MOBILE_UI)
    // draw hovering
    if (m_hoverValue > 0 && (m_checkValue == 0.0 || m_checkValue == 1.0)) {
        QRadialGradient rg = m_checkValue == 1.0 ? QRadialGradient(0.5, 0.2, 0.8) : QRadialGradient(0.5, 1.0, 1.5);
        QColor startColor(Qt::white);
        startColor.setAlpha((int)(255.0 * m_hoverValue));
        rg.setColorAt(0.0, startColor);
        rg.setColorAt(1.0, Qt::transparent);
        rg.setColorAt(0.0, startColor);
        rg.setColorAt(1.0, Qt::transparent);
        rg.setCoordinateMode(QGradient::ObjectBoundingMode);
        p.fillRect(0, 0, width(), height() - m_checkValue * (height() - 12) , rg);
    }

    // draw left/right lines
    if (m_borderFlags & 0x0001)
        p.fillRect(0, 0, 1, height(), QColor(230, 230, 230));
    if (m_borderFlags & 0x0002)
        p.fillRect(width() - 1, 0, 1, height(), Qt::white);
#endif

    // precalc text position and move painter
    QStyle * ss = m_checkable ? style() : 0;
    int indW = ss ? ss->pixelMetric(QStyle::PM_IndicatorWidth, 0, 0) : 0;
    int indH = ss ? ss->pixelMetric(QStyle::PM_IndicatorHeight, 0, 0) : 0;
    p.save();
    p.setFont(m_titleFont);
    QFontMetrics metrics(m_titleFont);
    QRect textRect = metrics.boundingRect(m_titleText);
//    int textHeight = textRect.height();
    int dx = 0;
    if (m_checkValue < 1.0) {
        qreal x1 = -textRect.top() + 2,
              x2 = (width() - textRect.width() - indW - 4) / 2;
        qreal y1 = height() - 2, //height() + textRect.width()) / 2,
              y2 = -textRect.top();
        p.translate(x1 + m_checkValue * (x2 - x1), y1 + m_checkValue * (y2 - y1));
        p.rotate(m_checkValue * 90 - 90);
    } else
        p.translate((width() - textRect.width() - indW - 4) / 2, -textRect.top());

    // draw checkbox indicator
    if (m_checkable && indW && indH) {
        QStyleOptionButton opt;
        opt.state = QStyle::State_Enabled;
        int styleOffset = (textRect.height() - indH) / 2;
        opt.rect = QRect(0, -indH + 4 - styleOffset, indW, indH);
        dx = indW + 4;
        opt.state |= m_checked ? QStyle::State_On : QStyle::State_Off;
        if (underMouse())
            opt.state |= QStyle::State_MouseOver;
        //p.setRenderHint(QPainter::Antialiasing, true);
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &p, this);
    }

    // draw text
    p.drawText(dx, 0, m_titleText);
    p.restore();
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

int GroupBoxWidget::calcMinWidth() const
{
    return qMax(QFontMetrics(m_titleFont).width(m_titleText) + 12, QWidget::sizeHint().width());
}

void GroupBoxWidget::updateDesign()
{
    if (m_collapsed) {
        // full collapse: no pre-work
    } else if (m_checkable && !m_checked) {
        // checkable off: label on left
        setContentsMargins(m_titleFont.pixelSize() + 8, 0, 0, 0);
        setCursor(Qt::PointingHandCursor);
    } else {
        // normal: label on top
        setContentsMargins(0, m_titleFont.pixelSize() + 8, 0, 0);
        setCursor(Qt::ArrowCursor);
    }

    // defer and accumulate layout calculations
    if (!m_redesignTimer) {
        m_redesignTimer = new QTimer(this);
        m_redesignTimer->setSingleShot(true);
        connect(m_redesignTimer, SIGNAL(timeout()), SLOT(slotAnimateDesign()));
    }
    m_redesignTimer->start();
}

void GroupBoxWidget::slotAnimateDesign()
{
    if (m_collapsed) {
        // full collapse: shrink to zero
        ANIMATE_PARAM("fixedWidth", 200, 0, true)
    } else if (m_checkable && !m_checked) {
        // checkable off: shrink to the target width
        ANIMATE_PARAM("fixedWidth", 200, m_titleFont.pixelSize() + 8, true);
        ANIMATE_PARAM("cAnim", 200, 0.0, false);
    } else {
        // normal: expand to the full size
        ANIMATE_PARAM("fixedWidth", 300, calcMinWidth(), true);
        ANIMATE_PARAM("cAnim", 200, 1.0, false);
    }

    // it was hidden in the constructor for not showing junk before initial layouting
    show();
}

void GroupBoxWidget::slotFinalizeDesign()
{
    if (m_collapsed) {
        // full collapse: zero width
        setFixedWidth(0);
    } else if (m_checkable && !m_checked) {
        // checkable off: left label
        setFixedWidth(m_titleFont.pixelSize() + 8);
        setCheckValue(0.0);
    } else  {
        // normal: layout driven
        setMinimumWidth(calcMinWidth());
        setMaximumWidth(QWIDGETSIZE_MAX);
        setCheckValue(1.0);
    }
}
