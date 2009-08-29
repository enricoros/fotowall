/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Tanguy Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "EmptyFrame.h"
#include "RenderOpts.h"
#include <QPainter>

quint32 EmptyFrame::frameClass() const
{
    return 0x0003;
}

void EmptyFrame::layoutButtons(QList<ButtonItem *> buttons, const QRect & frameRect) const
{
    const int spacing = 4;
    int left = frameRect.left();
    int top = frameRect.top();
    int right = frameRect.right();
    int bottom = frameRect.bottom();
    int offset = right;
    foreach (ButtonItem * button, buttons) {
        switch (button->buttonType()) {
            case ButtonItem::FlipH:
                button->setPos(right - button->width() / 2, (top + bottom) / 2);
                break;

            case ButtonItem::FlipV:
                button->setPos((left + right) / 2, top + button->height() / 2);
                break;

            default:
                button->setPos(offset - button->width() / 2, bottom - button->height() / 2);
                offset -= button->width() + spacing;
                break;
        }
    }
}

void EmptyFrame::layoutText(QGraphicsItem * textItem, const QRect & /*frameRect*/) const
{
    textItem->hide();
}

void EmptyFrame::paint(QPainter *painter, const QRect &rect, bool selected, bool /*opaqueContents*/)
{
    // draw aligned antialiased rect like in AbstractContent.cpp with !frame
    if (selected) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(RenderOpts::hiColor, 1.0));
        // FIXME: this draws OUTSIDE (but inside the safe 2px area)
        painter->drawRect(QRectF(rect).adjusted(-0.5, -0.5, +0.5, +0.5));
    }
}

QPixmap EmptyFrame::preview(int width, int height)
{
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    QPainter pixPainter(&pixmap);
    pixPainter.drawText(pixmap.rect(), Qt::AlignCenter, "X");
    pixPainter.end();
    return pixmap;
}
