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

#include "PixmapButton.h"

#include <QApplication>
#include <QPaintEvent>
#include <QPainter>

PixmapButton::PixmapButton(const QSize & fixedSize, QWidget * parent)
    : QAbstractButton(parent)
    , m_hovering(false)
{
    m_hoverFont = QApplication::font();
    m_hoverFont.setPixelSize(10);
    setFixedSize(fixedSize);
}

void PixmapButton::setPixmap(const QPixmap & pixmap)
{
    m_fixedPixmap = pixmap;
    update();
}

QPixmap PixmapButton::pixmap() const
{
    return m_fixedPixmap;
}

void PixmapButton::setHoverText(const QString &text)
{
    m_hoverText = text;
    update();
}

QString PixmapButton::hoverText() const
{
    return m_hoverText;
}

void PixmapButton::setFixedSize(const QSize & size)
{
    m_fixedSize = size;
    QAbstractButton::setFixedSize(m_fixedSize);
}

void PixmapButton::setFixedSize(int w, int h)
{
    setFixedSize(QSize(w, h));
}

QSize PixmapButton::fixedSize() const
{
    return m_fixedSize;
}

void PixmapButton::enterEvent(QEvent *)
{
    m_hovering = true;
    update();
}

void PixmapButton::leaveEvent(QEvent *)
{
    m_hovering = false;
    update();
}

void PixmapButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // draw background if hovering
    if (m_hovering)
        p.fillRect(rect(), palette().color(QPalette::Highlight));

    // draw pixmap, or placeholder
    if (m_fixedPixmap.isNull()) {
#if 0
        p.fillRect(rect().adjusted(2, 2, -2, -2), palette().color(QPalette::Button));
#endif
    } else {
        int offset = isDown() ? 1 : 0;
        p.drawPixmap( offset + (width() - m_fixedPixmap.width()) / 2,
                      offset + (height() - m_fixedPixmap.height()) / 2,
                      m_fixedPixmap);
    }

    // draw hover text, if any
    if (!m_hoverText.isEmpty()) {
#if 1
        const int h = 13;
        const int w = h * m_hoverText.size();
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(palette().color(QPalette::Highlight));
        QRectF rect(0, 0, w, h);
        p.drawEllipse(rect.adjusted(0.5, 0.5, -0.5, -0.5));
        p.setPen(palette().color(QPalette::HighlightedText));
        p.setFont(m_hoverFont);
        p.drawText(rect.toRect(), Qt::AlignCenter, m_hoverText);
#endif
    }

    // draw focus, if any
    if (hasFocus()) {
        p.setRenderHint(QPainter::Antialiasing, false);
        p.setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }
}
