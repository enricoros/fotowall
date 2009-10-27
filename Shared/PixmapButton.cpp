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

#include <QPaintEvent>
#include <QPainter>

PixmapButton::PixmapButton(const QSize & fixedSize, QWidget * parent)
    : QAbstractButton(parent)
    , m_hovering(false)
{
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

    // draw placeholder in place of pixmap
    if (m_fixedPixmap.isNull()) {
#if 0
        p.fillRect(rect().adjusted(2, 2, -2, -2), palette().color(QPalette::Button));
#endif
        return;
    }

    // draw pixmap
    int offset = isDown() ? 1 : 0;
    p.drawPixmap( offset + (width() - m_fixedPixmap.width()) / 2,
                  offset + (height() - m_fixedPixmap.height()) / 2,
                  m_fixedPixmap);
}
