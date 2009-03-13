/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "HelpItem.h"
#include "BrowserItem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "frames/FrameFactory.h"

HelpItem::HelpItem(QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_frame(FrameFactory::createFrame(0x1001 /*HARDCODED*/))
{
    // show fancy help in internal browser
    BrowserItem * bi = new BrowserItem(this);
    bi->setGeometry(m_frame->contentsRect(boundingRect().toRect()));
    bi->browse("qrc:/data/help.html");
    bi->setReadOnly(true);
}

HelpItem::~HelpItem()
{
    delete m_frame;
}

QRectF HelpItem::boundingRect() const
{
    return QRectF(-280, -160, 560, 320);
}

void HelpItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    m_frame->paint(painter, boundingRect().toRect(), false);
}

void HelpItem::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    emit closeMe();
}
