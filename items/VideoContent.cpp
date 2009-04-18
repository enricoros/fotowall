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

#include "VideoContent.h"
#include "VideoProvider.h"
#include "ButtonItem.h"
#include "RenderOpts.h"
#include "frames/Frame.h"
#include <QPainter>
#include <QTimer>

VideoContent::VideoContent(int input, QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("This is a mirror ;-)"));

    // initial pixmap
    setPixmap(QPixmap(":/data/add-video.png"));

    // start the video flow
    VideoProvider::instance()->connectInput(input, this, SLOT(setPixmap(const QPixmap &)));
}

VideoContent::~VideoContent()
{
    // stop the video flow
    VideoProvider::instance()->disconnectReceiver(this);
}

void VideoContent::setPixmap(const QPixmap & pixmap)
{
    m_pixmap = pixmap;
    if (!beingTransformed())
        m_cachedPixmap = QPixmap();
    //if (keepRatio)
    //    adjustSize();
    update();
    GFX_CHANGED();
}

QPixmap VideoContent::renderAsBackground(const QSize & size) const
{
    if (m_pixmap.isNull())
        return AbstractContent::renderAsBackground(size);
    return m_pixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

int VideoContent::contentHeightForWidth(int width) const
{
    if (m_pixmap.width() < 1)
        return -1;
    return (m_pixmap.height() * width) / m_pixmap.width();
}

bool VideoContent::contentOpaque() const
{
    return true;
}

void VideoContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void VideoContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    // skip if no photo
    if (m_pixmap.isNull())
        return;

    // blit if opaque picture
#if QT_VERSION >= 0x040500
    // disabled for 4.5 too, since it relies on raster.
    //painter->setCompositionMode(QPainter::CompositionMode_Source);
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
//    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}
