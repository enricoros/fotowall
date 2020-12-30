/*
    PosteRazor - Make your own poster!
    Copyright (C) 2005-2009 by Alessandro Portale
    http://posterazor.sourceforge.net/

    This file is part of PosteRazor

    PosteRazor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PosteRazor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PosteRazor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "paintcanvas.h"
#include <QImage>
#include <QPainter>
#include <QPainterPath>

PaintCanvas::PaintCanvas(QWidget *parent)
    : QWidget(parent)
    , m_qPainter(NULL)
    , m_state(QLatin1String("image"))
{
}

void PaintCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    m_qPainter = &painter;
    m_qPainter->setRenderHint(QPainter::Antialiasing);
    emit needsPaint(this, m_state);
    m_qPainter = NULL;
}

void PaintCanvas::drawFilledRect(const QRectF &rect, const QBrush &brush)
{
    m_qPainter->fillRect(rect, brush);
}

QSizeF PaintCanvas::size() const
{
    return QWidget::size();
}

void PaintCanvas::setImage(const QImage &image)
{
    m_image = image;
    repaint();
}

void PaintCanvas::drawImage(const QRectF &rect)
{
    double widthResizeFactor = rect.width()/(double)m_image.width();
    m_qPainter->setRenderHint(QPainter::SmoothPixmapTransform, widthResizeFactor < 2.75);
    m_qPainter->drawImage(rect, m_image);
}

void PaintCanvas::setState(const QString &state)
{
    m_state = state;
    repaint();
}

void PaintCanvas::drawOverlayText(const QPointF &position, int flags, int size, const QString &text)
{
    Q_UNUSED(flags)
    if (size < 8)
        return;
    QFont font;
    font.setPixelSize(size);
    m_qPainter->save();
    m_qPainter->setOpacity(0.70);
    const QColor fontColor(0xeeeeee);
    QFontMetricsF fontMetrics(font);
    const qreal textWidth = fontMetrics.width(text);
    const QPointF fontOffset(QPointF(-textWidth / 2, fontMetrics.xHeight() * 1.5));
    if (size > 35) {
        m_qPainter->setPen(0x656565);
        m_qPainter->setBrush(fontColor);
        QPainterPath textPath;
        textPath.addText(position + fontOffset, font, text);
        m_qPainter->drawPath(textPath);
    } else {
        m_qPainter->setPen(fontColor);
        m_qPainter->setRenderHint(QPainter::TextAntialiasing);
        m_qPainter->setFont(font);
        m_qPainter->drawText(position + fontOffset, text);
    }
    m_qPainter->restore();
}
