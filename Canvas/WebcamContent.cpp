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
#include <QSvgRenderer>
#include <QTimer>

WebcamContent::WebcamContent(int input, QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_input(input)
    , m_still(false)
    , m_dummyRenderer(0)
    , m_broken(false)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("This is a mirror ;-)"));

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
    else
        m_broken = true;
}

WebcamContent::~WebcamContent()
{
    delete m_dummyRenderer;

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

bool WebcamContent::fromXml(QDomElement & contentElement)
{
    AbstractContent::fromXml(contentElement);

    // load video properties
    // ... nothing to do here...
    return true;
}

void WebcamContent::toXml(QDomElement & contentElement) const
{
    AbstractContent::toXml(contentElement);
    contentElement.setTagName("webcam");
    contentElement.setAttribute("input", m_input);

    // nothing to save here... (maybe the still pic?)
}

void WebcamContent::drawContent(QPainter * painter, const QRect & targetRect)
{
#if 0 && QT_VERSION >= 0x040500
    // delay rendering until the element has been fully shown
    if (opacity() < 1.0)
        return;
#endif

    bool smoothOn = RenderOpts::HQRendering ? true : !beingTransformed();

    // select the pixmap to draw
    QPixmap * pixmap = &m_pixmap;
    if (pixmap->isNull()) {
        // regenerate the dummy pixmap if needed
        if (m_dummyPixmap.isNull() || (m_dummyPixmap.size() != targetRect.size() && smoothOn)) {
            m_dummyPixmap = QPixmap(targetRect.size());
            m_dummyPixmap.fill(m_broken ? Qt::red : Qt::white);
            QPainter dummyPainter(&m_dummyPixmap);
            if (!m_dummyRenderer)
                m_dummyRenderer = new QSvgRenderer(QString(":/data/webcam-loading.svg"));
            m_dummyRenderer->render(&dummyPainter);
        }
        // draw the dummy pixmap
        pixmap = &m_dummyPixmap;
    }

    // blit if opaque picture
#if QT_VERSION >= 0x040500
    // disabled for 4.5 too, since it relies on raster.
    //painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

    // draw high-resolution photo when exporting png
    painter->setRenderHint(QPainter::SmoothPixmapTransform, smoothOn);
    painter->drawPixmap(targetRect, *pixmap);

#if QT_VERSION >= 0x040500
    //painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

QPixmap WebcamContent::toPixmap(const QSize & size, Qt::AspectRatioMode ratio)
{
    if (!m_pixmap.isNull())
        return ratioScaledPixmap(&m_pixmap, size, ratio);
    return AbstractContent::toPixmap(size, ratio);
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

void WebcamContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit requestBackgrounding();
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
