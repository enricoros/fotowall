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

#include "PictureContent.h"
#include "ButtonItem.h"
#include "CPixmap.h"
#include "RenderOpts.h"
#include "frames/Frame.h"
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QNetworkReply>
#include <QPainter>
#include <QUrl>

PictureContent::PictureContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_photo(0)
    , m_opaquePhoto(false)
    , m_progress(0)
    , m_netWidth(0)
    , m_netHeight(0)
    , m_netReply(0)
{
    // enable frame text
    setFrameTextEnabled(true);
    setFrameText(tr("..."));

    // add flipping buttons
    ButtonItem * bFlipH = new ButtonItem(ButtonItem::FlipH, Qt::blue, QIcon(":/data/action-flip-horizontal.png"), this);
    bFlipH->setToolTip(tr("Flip horizontally"));
    bFlipH->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    connect(bFlipH, SIGNAL(clicked()), this, SIGNAL(flipHorizontally()));
    addButtonItem(bFlipH);

    ButtonItem * bFlipV = new ButtonItem(ButtonItem::FlipV, Qt::blue, QIcon(":/data/action-flip-vertical.png"), this);
    bFlipV->setToolTip(tr("Flip vertically"));
    bFlipV->setFlag(QGraphicsItem::ItemIgnoresTransformations, false);
    addButtonItem(bFlipV);
    connect(bFlipV, SIGNAL(clicked()), this, SIGNAL(flipVertically()));
}

PictureContent::~PictureContent()
{
    dropNetworkConnection();
    delete m_photo;
}

bool PictureContent::loadPhoto(const QString & fileName, bool keepRatio, bool setName)
{
    dropNetworkConnection();
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = 0;
    m_filePath = QString();
    m_netWidth = 0;
    m_netHeight = 0;

    m_photo = new CPixmap(fileName);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        return false;
    }

    m_opaquePhoto = !m_photo->hasAlpha();
    m_filePath = fileName;
    if (keepRatio)
        resetContentsRatio();
    if (setName) {
        QString string = QFileInfo(fileName).fileName().section('.', 0, 0);
        string = string.mid(0, 10);
        setFrameText(string + tr("..."));
    }
    update();
    GFX_CHANGED();
    return true;
}

bool PictureContent::loadFromNetwork(QNetworkReply * reply, const QString & title, int width, int height)
{
    dropNetworkConnection();
    delete m_photo;
    m_cachedPhoto = QPixmap();
    m_opaquePhoto = false;
    m_photo = 0;
    m_filePath = QString();
    m_netWidth = width;
    m_netHeight = height;
    m_netReply = reply;

    // set title
    if (!title.isEmpty())
        setFrameText(title.mid(0, 10) + tr("..."));

    // Immediate Decode: just handle the reply if done
#if QT_VERSION >= 0x040600
    if (m_netReply->isFinished())
        return slotLoadNetworkData();
#endif

    // Deferred Decode: listen to the network job
    m_progress = 0.01;
    connect(m_netReply, SIGNAL(finished()), this, SLOT(slotLoadNetworkData()));
    connect(m_netReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(slotNetworkProgress(qint64,qint64)));

    // reset size, if got the network one
    if (m_netWidth > 0 && m_netHeight > 0)
        resetContentsRatio();
    return true;
}

void PictureContent::addEffect(const CEffect & effect)
{
    if (!m_photo)
        return;
    m_photo->addEffect(effect);
    m_cachedPhoto = QPixmap();
    update();
    GFX_CHANGED();
}

bool PictureContent::fromXml(QDomElement & pe)
{
    AbstractContent::fromXml(pe);

    // load picture properties
    QString name = pe.firstChildElement("name").text();
    QString path = pe.firstChildElement("path").text();
    bool ok = loadPhoto(path);
    if (ok) {
        QDomElement effectsE = pe.firstChildElement("effects");
        for (QDomElement effectE = effectsE.firstChildElement("effect"); effectE.isElement(); effectE = effectE.nextSiblingElement("effect")) {
            CEffect fx;
            fx.effect = (CEffect::Effect)effectE.attribute("type").toInt();
            fx.param = effectE.attribute("param").toDouble();
            addEffect(fx);
        }
    }
    return ok;
}

void PictureContent::toXml(QDomElement & pe) const
{
    AbstractContent::toXml(pe);
    pe.setTagName("picture");

    // save picture properties
    QDomDocument doc = pe.ownerDocument();
    QDomElement domElement;
    QDomText text;

    // Save image path
    domElement = doc.createElement("path");
    pe.appendChild(domElement);
    text = doc.createTextNode(m_filePath);
    domElement.appendChild(text);

    // Save the effects
    domElement = doc.createElement("effects");
    pe.appendChild(domElement);
    QString effectStr;
    if (m_photo) {
        foreach (const CEffect & effect, m_photo->effects()) {
            QDomElement effectElement = doc.createElement("effect");
            effectElement.setAttribute("type", effect.effect);
            effectElement.setAttribute("param", effect.param);
            domElement.appendChild(effectElement);
        }
    }
}

QPixmap PictureContent::renderAsBackground(const QSize & size, bool keepAspect) const
{
    if (m_photo)
        return m_photo->scaled(size, keepAspect ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return AbstractContent::renderAsBackground(size, keepAspect);
}

int PictureContent::contentHeightForWidth(int width) const
{
    if (m_netWidth > 0 && m_netHeight > 0)
        return (m_netHeight * width) / m_netWidth;
    if (!m_photo || m_photo->width() < 1)
        return -1;
    return (m_photo->height() * width) / m_photo->width();
}

bool PictureContent::contentOpaque() const
{
    return m_opaquePhoto;
}

void PictureContent::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // load the first valid picture
    foreach (QUrl url, event->mimeData()->urls()) {
        if (loadPhoto(url.toLocalFile(), true, false)) {
            event->accept();
            return;
        }
    }
}

void PictureContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    emit backgroundMe();
    QGraphicsItem::mouseDoubleClickEvent(event);
}

void PictureContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // paint parent
    AbstractContent::paint(painter, option, widget);

    // draw progress
    if (m_progress > 0.0 && m_progress < 1.0) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::blue);
        painter->drawPie(QRect(-10, -10, 20, 20), 90 * 16, (int)(-5760.0 * m_progress));
    }

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

void PictureContent::dropNetworkConnection()
{
    if (m_netReply) {
        m_netReply->disconnect(0,0,0);
#if QT_VERSION >= 0x040600
        if (!m_netReply->isFinished())
            m_netReply->abort();
#endif
        m_netReply->deleteLater();
        m_netReply = 0;
    }
    m_progress = 0.0;
}

bool PictureContent::slotLoadNetworkData()
{
    // get the data
    QByteArray replyData = m_netReply->readAll();
    dropNetworkConnection();

    // make the QImage from data
    QImage image = QImage::fromData(replyData);
    if (image.isNull())
        return false;

    // update contents
    m_netWidth = image.width();
    m_netHeight = image.height();
    resetContentsRatio();
    m_progress = 1.0;
    m_photo = new CPixmap(image);
    if (m_photo->isNull()) {
        delete m_photo;
        m_photo = 0;
        return false;
    }
    m_opaquePhoto = !m_photo->hasAlpha();
    update();
    GFX_CHANGED();
    return true;
}

void PictureContent::slotNetworkProgress(qint64 a, qint64 b)
{
    m_progress = b > 0 ? (double)a / (double)b : 0.0;
    update();
}
