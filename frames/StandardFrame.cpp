/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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

#include "StandardFrame.h"
#include "RenderOpts.h"
#include <QLinearGradient>
#include <QPainter>

//#define SCALE_X     (- 29 - 5)
//#define SCALE_Y     (- 8 -5)
//#define ROTATE_X    (- 8 - 5)
//#define ROTATE_Y    (- 8 -5)
#define FW_MARGIN   10
#define FW_LABH     30

quint32 StandardFrame::frameClass() const
{
    return 0x0001;
}

QRect StandardFrame::frameRect(const QRect & contentsRect) const
{
    return contentsRect.adjusted(-FW_MARGIN, -FW_MARGIN, FW_MARGIN, FW_MARGIN + FW_LABH);
}

void StandardFrame::layoutButtons(QList<ButtonItem *> buttons, const QRect & frameRect) const
{
    const int spacing = 4;
    int left = frameRect.left() + FW_MARGIN;
    int top = frameRect.top() + FW_MARGIN;
    int right = frameRect.right() - FW_MARGIN;
    int bottom = frameRect.bottom() - FW_MARGIN;
    int offset = right;
    foreach (ButtonItem * button, buttons) {
        switch (button->buttonType()) {
            case ButtonItem::FlipH:
                button->setPos(right - button->width() / 2, (top + bottom - button->height()) / 2);
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

void StandardFrame::layoutText(QGraphicsItem * textItem, const QRect & frameRect) const
{
    textItem->show();
    textItem->setPos(frameRect.left() + FW_MARGIN, frameRect.bottom() - FW_MARGIN - FW_LABH + FW_MARGIN);
}

void StandardFrame::paint(QPainter * painter, const QRect & frameRect, bool selected, bool /*opaqueContents*/)
{
    //painter->fillRect(boundingRect(), QColor(0,0,0,64));
    QLinearGradient lg(0, frameRect.top(), 0, frameRect.height() / 2);
    lg.setColorAt(0.0, selected ? RenderOpts::hiColor.darker() : QColor(128,128,128, 200));
    lg.setColorAt(1.0, selected ? RenderOpts::hiColor.lighter() : QColor(255,255,255, 200));
    painter->fillRect(frameRect, lg);
    //painter->fillRect(boundingRect().adjusted(5, 5, -5, -5), lg);
}
