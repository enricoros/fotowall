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

#include "TextContent.h"
#include "ButtonItem.h"
#include "CPixmap.h"
#include "RenderOpts.h"
#include "frames/Frame.h"
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QUrl>

TextContent::TextContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent)
    , m_textItem(0)
{
    m_textItem = new QGraphicsTextItem("ciao", this);
    m_textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    QFont f("Sans Serif");
    //f.setPointSizeF(7.5);
    m_textItem->setFont(f);
}

TextContent::~TextContent()
{
    delete m_textItem;
}

void TextContent::save(QDataStream & data) const
{
    AbstractContent::save(data);
}

bool TextContent::restore(QDataStream & data)
{
    return AbstractContent::restore(data);
}

void TextContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    // skip if no photo
    //m_textItem->paint(painter, option, widget);
}
