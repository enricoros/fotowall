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

#include "WebcamContent.h"

#include "Frames/Frame.h"
#include "Shared/RenderOpts.h"
#include "Shared/VideoProvider.h"
#include "ButtonItem.h"

#include <QPainter>
#include <QTimer>

WebcamContent::WebcamContent(int input, QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_input(input)
    , m_still(false)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("This is a mirror ;-)"));

    // initial pixmap
    setPixmap(QPixmap(":/data/insert-camera.png"));

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
    if (input >= 0 && input < VideoProvider::instance()->inputCount())
        VideoProvider::instance()->connectInput(input, this, SLOT(setPixmap(const QPixmap &)));
    else {
        // TODO: show a still picture about a "broken connection to the camera"
    }
}

WebcamContent::~WebcamContent()
{
    // stop the video flow
    VideoProvider::instance()->disconnectReceiver(this);
}

void WebcamContent::setPixmap(const QPixmap & pixmap)
{
    if (m_still)
        return;
    m_pixmap = pixmap;
    //if (keepRatio)
    //    adjustSize();
    update();
    GFX_CHANGED();

    // notify image change
    emit contentChanged();
}

bool WebcamContent::fromXml(QDomElement & pe)
{
    AbstractContent::fromXml(pe);

    // load video properties
    // ... nothing to do here...
    return true;
}

void WebcamContent::toXml(QDomElement & pe) const
{
    AbstractContent::toXml(pe);
    pe.setTagName("webcam");
    pe.setAttribute("input", m_input);

    // nothing to save here... (maybe the still pic?)
}

void WebcamContent::drawContent(QPainter * painter, const QRect & targetRect)
{
    // skip if no photo
    if (m_pixmap.isNull())
        return;

    // blit if opaque picture
#if QT_VERSION >= 0x040500
    // disabled for 4.5 too, since it relies on raster.
    //painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

    // draw high-resolution photo when exporting png
    bool smoothOn = RenderOpts::HQRendering ? true : !beingTransformed();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, smoothOn);
    painter->drawPixmap(targetRect, m_pixmap);

#if QT_VERSION >= 0x040500
//    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

QPixmap WebcamContent::toPixmap(const QSize & size, Qt::AspectRatioMode ratio)
{
    if (!m_pixmap.isNull())
        return ratioScaledPixmap(&m_pixmap, size, ratio);
    return QPixmap();
}

int WebcamContent::contentHeightForWidth(int width) const
{
    if (m_pixmap.width() < 1)
        return -1;
    return (m_pixmap.height() * width) / m_pixmap.width();
}

bool WebcamContent::contentOpaque() const
{
    return true;
}

void WebcamContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void WebcamContent::slotToggleStill()
{
    m_still = !m_still;
}

void WebcamContent::slotToggleSwap()
{
    bool swapState = !VideoProvider::instance()->swapped(m_input);
    VideoProvider::instance()->setSwapped(m_input, swapState);
}
