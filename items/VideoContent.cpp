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
    , m_input(input)
    , m_still(false)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("This is a mirror ;-)"));

    // initial pixmap
    setPixmap(QPixmap(":/data/add-video.png"));

    // add swap button
    ButtonItem * bSwap = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-flip-horizontal.png"), this);
    bSwap->setToolTip(tr("Mirror Video"));
    connect(bSwap, SIGNAL(clicked()), this, SLOT(slotToggleSwap()));
    addButtonItem(bSwap);

    // add snapshot button
    ButtonItem * bStill = new ButtonItem(ButtonItem::Control, Qt::blue, QIcon(":/data/action-snapshot.png"), this);
    bStill->setToolTip(tr("Still picture"));
    connect(bStill, SIGNAL(clicked()), this, SLOT(slotToggleStill()));
    addButtonItem(bStill);

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
    if (m_still)
        return;
    m_pixmap = pixmap;
    //if (keepRatio)
    //    adjustSize();
    update();
    GFX_CHANGED();
}

bool VideoContent::fromXml(QDomElement & pe)
{
    AbstractContent::fromXml(pe);

    // load video properties
    // ... nothing to do here...
    return true;
}

void VideoContent::toXml(QDomElement & pe) const
{
    AbstractContent::toXml(pe);

    // nothing to save here... (maybe the still pic?)
}

QPixmap VideoContent::renderAsBackground(const QSize & size, bool keepAspect) const
{
    if (m_pixmap.isNull())
        return AbstractContent::renderAsBackground(size, keepAspect);
    return m_pixmap.scaled(size, keepAspect ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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
    bool smoothOn = RenderOpts::HQRendering ? true : !beingTransformed();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, smoothOn);
    painter->drawPixmap(targetRect, m_pixmap);

#if QT_VERSION >= 0x040500
//    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

void VideoContent::slotToggleStill()
{
    m_still = !m_still;
}

void VideoContent::slotToggleSwap()
{
    bool swapState = !VideoProvider::instance()->swapped(m_input);
    VideoProvider::instance()->setSwapped(m_input, swapState);
}
