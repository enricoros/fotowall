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

#include "PixmapContent.h"
#include "ButtonItem.h"
#include "CPixmap.h"
#include "RenderOpts.h"
#include "frames/Frame.h"
#include <QGraphicsScene>
#include <QPainter>

PixmapContent::PixmapContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("This is a mirror ;-)"));
}

PixmapContent::~PixmapContent()
{
}

void PixmapContent::setPixmp(const QPixmap & pixmap, bool keepRatio)
{
    m_pixmap = pixmap;
    if (!beingTransformed())
        m_cachedPixmap = QPixmap();
    if (keepRatio)
        adjustSize();
    update();
    GFX_CHANGED();
}

QPixmap PixmapContent::renderAsBackground(const QSize & size) const
{
    if (m_pixmap.isNull())
        return AbstractContent::renderAsBackground(size);
    return m_pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

int PixmapContent::contentHeightForWidth(int width) const
{
    if (m_pixmap.width() < 1)
        return -1;
    return (m_pixmap.height() * width) / m_pixmap.width();
}

bool PixmapContent::contentOpaque() const
{
    return true;
}

void PixmapContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void PixmapContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    // skip if no photo
    if (m_pixmap.isNull())
        return;

    // blit if opaque picture
#if QT_VERSION >= 0x040500
    //disabled for 4.5 too, since it relies on raster.
    //if (m_opaquePhoto)
    //    painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

    // draw high-resolution photo when exporting png
    QRect targetRect = contentsRect();
    // TODO: fix drawing
//    if (RenderOpts::HQRendering) {
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect, m_pixmap);
        return;
  //  }

    // draw photo using caching and deferred rescales
    if (beingTransformed()) {
        if (!m_cachedPixmap.isNull())
            painter->drawPixmap(targetRect, m_cachedPixmap);
    } else {
        if (m_cachedPixmap.isNull() || m_cachedPixmap.size() != targetRect.size())
            m_cachedPixmap = m_pixmap.scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(targetRect.topLeft(), m_cachedPixmap);
    }

#if QT_VERSION >= 0x040500
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}
