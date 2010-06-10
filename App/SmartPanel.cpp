/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009-2010 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "SmartPanel.h"
#include <QApplication>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QDebug>

#define SP_MARGIN 2
#define SP_SPACING 2

SmartPanel::SmartPanel(const QString &label, QWidget *embeddedWidget, QWidget *parent)
  : QWidget(parent)
  , m_text(label)
  , m_basePos(0, 0)
  , m_embeddedWidget(embeddedWidget)
{
    m_embeddedWidget->setParent(this);
    m_embeddedWidget->installEventFilter(this);
    updateLayout();
}

SmartPanel::~SmartPanel()
{
    m_embeddedWidget->removeEventFilter(this);
}

void SmartPanel::setBasePos(const QPoint &pos)
{
    if (m_basePos != pos) {
        m_basePos = pos;
        updateLayout();
    }
}

void SmartPanel::setPanelState(qreal state)
{
    if (state != m_panelState) {
        m_panelState = state;
        move(m_basePos + QPoint(0, verticalOffset()));
    }
}

int SmartPanel::verticalOffset() const
{
    if (!m_panelState)
        return -m_labelRect.height();
    if (m_panelState > 0)
        return -m_labelRect.height() - (int)((qreal)(SP_SPACING + m_embeddedWidget->height() + SP_MARGIN) * m_panelState);
    return -(int)((qreal)m_labelRect.height() * (1.0 + m_panelState));
}

void SmartPanel::updateLayout()
{
    // calc text box size
    QFontMetrics fm = QApplication::fontMetrics();
    int textWidth = fm.width(m_text) + 20;
    m_labelRect = QRect(0, 0, textWidth, 20);

    // calc overall size
    int minWidth = qMax(m_labelRect.width(), SP_MARGIN + SP_MARGIN + m_embeddedWidget->width());
    int minHeight = m_labelRect.height() + SP_SPACING + m_embeddedWidget->height() + SP_MARGIN;
    m_minSize = QSize(minWidth, minHeight);

    // reposition embedded widget
    m_embeddedWidget->move(SP_MARGIN, m_labelRect.height() + SP_SPACING);

    // move and resize me
    resize(m_minSize);
    move(m_basePos + QPoint(0, verticalOffset()));
}

bool SmartPanel::eventFilter(QObject * object, QEvent * event)
{
    if (object == m_embeddedWidget && event->type() == QEvent::Resize)
        updateLayout();
    return false;
}

void SmartPanel::mousePressEvent(QMouseEvent *event)
{
    if (m_labelRect.contains(event->pos())) {
        if (m_panelState <= 0.01) {
            QPropertyAnimation * ani = new QPropertyAnimation(this, "panelState", this);
            ani->setEndValue((qreal)1.0);
            ani->setEasingCurve(QEasingCurve::OutQuad);
            ani->setDuration(200);
            ani->start(QAbstractAnimation::DeleteWhenStopped);
            emit panelRaising();
        } else {
            QPropertyAnimation * ani = new QPropertyAnimation(this, "panelState", this);
            ani->setEndValue((qreal)0.0);
            ani->setEasingCurve(QEasingCurve::OutQuad);
            ani->setDuration(200);
            ani->start(QAbstractAnimation::DeleteWhenStopped);
            emit panelLowering();
        }
    }
}

void SmartPanel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QLinearGradient lg(0, 0, 0, height());
    lg.setColorAt(0.0, Qt::lightGray);
    lg.setColorAt(1.0, Qt::gray);
    p.fillRect(m_labelRect, lg);
    p.fillRect(0, m_labelRect.bottom(), width(), height() - m_labelRect.bottom(), lg);
    p.drawText(m_labelRect, Qt::AlignCenter, m_text);
}
