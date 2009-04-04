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

#include "PictureItem.h"
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QPainter>
#include <QUrl>
#include "CPixmap.h"
#include "RenderOpts.h"
#include "frames/Frame.h"

PictureItem::PictureItem(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContentItem(scene, parent)
    , m_photo(0)
    , m_opaquePhoto(false)
{
    setFrameTextEnabled(true);
    setFrameText(tr("..."));
}

PictureItem::~PictureItem()
{
    delete m_photo;
}

bool PictureItem::loadPhoto(const QString & fileName, bool keepRatio, bool setName)
{
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = new CPixmap(fileName);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        m_fileName = QString();
        return false;
    }
    m_opaquePhoto = !m_photo->hasAlpha();
    m_fileName = fileName;
    if (keepRatio)
        adjustSize();
    if (setName) {
        QString string = QFileInfo(fileName).fileName().section('.', 0, 0);
        string = string.mid(0, 10);
        setFrameText(string + tr("..."));
    }
    update();
    GFX_CHANGED();
    return true;
}

void PictureItem::setEffect(int effectClass)
{
    // apply effect to pixmap
    switch (effectClass) {
        case 0:
            m_photo->invertColors();
            break;
        case 1:
            m_photo->toNVG();
            break;
        case 2:
            m_photo->toBlackAndWhite();
            break;
        case 3:
            m_photo->noEffects();
            break;
        default:
            qWarning("PictureItem::setEffect: effect %d is not implemented", effectClass);
            return;
    }

    // invalidate cache and repaint
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED();
}

QPixmap PictureItem::renderPhoto(const QSize & size) const
{
    if (m_photo)
        return m_photo->scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return QPixmap();
}

void PictureItem::save(QDataStream & data) const
{
    AbstractContentItem::save(data);
    data << m_fileName;
}

bool PictureItem::restore(QDataStream & data)
{
    AbstractContentItem::restore(data);
    QString fileName;
    data >> fileName;
    bool ok = loadPhoto(fileName);
    return ok;
}

int PictureItem::contentHeightForWidth(int width) const
{
    if (!m_photo || m_photo->width() < 1)
        return -1;
    return (m_photo->height() * width) / m_photo->width();
}

bool PictureItem::contentOpaque() const
{
    return m_opaquePhoto;
}

void PictureItem::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // load the first valid picture
    foreach (QUrl url, event->mimeData()->urls()) {
        if (loadPhoto(url.toLocalFile(), true, false)) {
            event->accept();
            return;
        }
    }
}

void PictureItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void PictureItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContentItem::paint(painter, option, widget);

    // skip if no photo
    if (!m_photo)
        return;

    // blit if opaque picture
#if QT_VERSION >= 0x040500
    //disabled for 4.5 too, since it relies on raster.
    //if (m_opaquePhoto)
    //    painter->setCompositionMode(QPainter::CompositionMode_Source);
#endif

    // draw high-resolution photo when exporting png
    QRect targetRect = contentsRect();
    if (RenderOpts::HQRendering) {
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect, *m_photo);
        return;
    }

    // draw photo using caching and deferred rescales
    if (beingTransformed()) {
        if (!m_cachedPhoto.isNull())
            painter->drawPixmap(targetRect, m_cachedPhoto);
    } else {
        if (m_cachedPhoto.isNull() || m_cachedPhoto.size() != targetRect.size())
            m_cachedPhoto = m_photo->scaled(targetRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->setRenderHints(QPainter::SmoothPixmapTransform);
        painter->drawPixmap(targetRect.topLeft(), m_cachedPhoto);
    }

#if QT_VERSION >= 0x040500
//    if (m_opaquePhoto)
//        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
#endif
}

void PictureItem::slotFlipHorizontally()
{
    // delete the Photo and and recreate an H-mirrored one
    CPixmap * oldPhoto = m_photo;
    m_photo = new CPixmap(CPixmap::fromImage(oldPhoto->toImage().mirrored(true, false)));
    delete oldPhoto;
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED();
}

void PictureItem::slotFlipVertically()
{
    // delete the Photo and and recreate a V-mirrored one
    CPixmap * oldPhoto = m_photo;
    m_photo = new CPixmap(CPixmap::fromImage(oldPhoto->toImage().mirrored(false, true)));
    delete oldPhoto;
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED();
}
