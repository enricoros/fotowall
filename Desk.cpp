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

#include "Desk.h"
#include "ColorPickerItem.h"
#include "PictureItem.h"
#include "PicturePropertiesItem.h"
#include "frames/FrameFactory.h"
#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <QImageReader>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QFile>

static int zLevel = 0;

#define COLORPICKER_W 200
#define COLORPICKER_H 150

Desk::Desk(QObject * parent)
    : QGraphicsScene(parent)
    , m_backPicture(0)
{
    // create colorpickers
    m_titleColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_titleColorPicker->setColor(Qt::gray);
    m_titleColorPicker->setAnimated(true);
    m_titleColorPicker->setAnchor(ColorPickerItem::AnchorTop);
    m_titleColorPicker->setZValue(10000);
    connect(m_titleColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotTitleColorChanged()));
    addItem(m_titleColorPicker);

    m_foreColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_foreColorPicker->setColor(QColor(128, 128, 128));
    m_foreColorPicker->setAnimated(true);
    m_foreColorPicker->setAnchor(ColorPickerItem::AnchorTopLeft);
    m_foreColorPicker->setZValue(10000);
    connect(m_foreColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotForeColorChanged()));
    addItem(m_foreColorPicker);

    m_grad1ColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_grad1ColorPicker->setColor(QColor(192, 192, 192));
    m_grad1ColorPicker->setAnimated(true);
    m_grad1ColorPicker->setAnchor(ColorPickerItem::AnchorTopRight);
    m_grad1ColorPicker->setZValue(10000);
    connect(m_grad1ColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotGradColorChanged()));
    addItem(m_grad1ColorPicker);

    m_grad2ColorPicker = new ColorPickerItem(COLORPICKER_W, COLORPICKER_H, 0);
    m_grad2ColorPicker->setColor(QColor(80, 80, 80));
    m_grad2ColorPicker->setAnimated(true);
    m_grad2ColorPicker->setAnchor(ColorPickerItem::AnchorBottomRight);
    m_grad2ColorPicker->setZValue(10000);
    connect(m_grad2ColorPicker, SIGNAL(colorChanged(const QColor&)), this, SLOT(slotGradColorChanged()));
    addItem(m_grad2ColorPicker);
}

Desk::~Desk()
{
    delete m_titleColorPicker;
    delete m_foreColorPicker;
    delete m_grad1ColorPicker;
    delete m_grad2ColorPicker;
    qDeleteAll(m_pictures);
    m_pictures.clear();
    m_backPicture = 0;
}


void Desk::resize(const QSize & size)
{
    // relayout contents
    m_size = size;
    m_rect = QRectF(0, 0, m_size.width(), m_size.height());
    m_titleColorPicker->setPos((size.width() - COLORPICKER_W) / 2.0, 10);
    m_grad1ColorPicker->setPos(size.width() - COLORPICKER_W, 0);
    m_grad2ColorPicker->setPos(size.width() - COLORPICKER_W, size.height() - COLORPICKER_H);

    // ensure visibility
    foreach (PictureItem * picture, m_pictures)
        picture->ensureVisible(m_rect);
    foreach (PicturePropertiesItem * properties, m_properties)
        properties->keepInBoundaries(m_rect.toRect());

    // change my rect
    setSceneRect(m_rect);
}

void Desk::save(QDataStream & data) const
{
    // save own data
    data << m_titleColorPicker->color();
    data << m_foreColorPicker->color();
    data << m_grad1ColorPicker->color();
    data << m_grad2ColorPicker->color();
    data << m_titleText;

    // save the photos
    data << m_pictures.size();
    foreach (PictureItem * foto, m_pictures)
        foto->save(data);

    // TODO: save background
}

void Desk::restore(QDataStream & data)
{
    // restore own data
    QColor color;
    data >> color;
    m_titleColorPicker->setColor(color);
    data >> color;
    m_foreColorPicker->setColor(color);
    data >> color;
    m_grad1ColorPicker->setColor(color);
    data >> color;
    m_grad2ColorPicker->setColor(color);
    data >> m_titleText;

    // FIXME: restore background

    // restore the photos
    qDeleteAll(m_pictures);
    m_pictures.clear();
    m_backPicture = 0;
    int photos = 0;
    data >> photos;
    for (int i = 0; i < photos; i++) {
        // HACK: unify the loading code (1)
        PictureItem * p = new PictureItem(this);
        //p->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        connect(p, SIGNAL(configureMe(const QPoint &)), this, SLOT(slotConfigurePicture(const QPoint &)));
        connect(p, SIGNAL(backgroundMe()), this, SLOT(slotBackgroundPicture()));
        connect(p, SIGNAL(raiseMe()), this, SLOT(slotRaisePicture()));
        connect(p, SIGNAL(deleteMe()), this, SLOT(slotDeletePicture()));
        if (!p->restore(data)) {
            delete p;
            continue;
        }
        m_pictures.append(p);
    }

    update();
}

void Desk::loadPictures(const QStringList & fileNames)
{
    double delta = 0;
    foreach (const QString & localFile, fileNames) {
        if (!QFile::exists(localFile))
            continue;

        // HACK: unify the loading code (2)
        PictureItem * p = new PictureItem(this);
        //p->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        connect(p, SIGNAL(configureMe(const QPoint &)), this, SLOT(slotConfigurePicture(const QPoint &)));
        connect(p, SIGNAL(backgroundMe()), this, SLOT(slotBackgroundPicture()));
        connect(p, SIGNAL(raiseMe()), this, SLOT(slotRaisePicture()));
        connect(p, SIGNAL(deleteMe()), this, SLOT(slotDeletePicture()));
        p->setPos(sceneRect().center().toPoint() + QPointF(delta, delta) );
        p->setZValue(++zLevel);
        if (!p->loadPhoto(localFile, true, true)) {
            delete p;
            continue;
        }
        p->show();
        m_pictures.append(p);
        delta += 30;
    }
}


/// Drag & Drop pictures
void Desk::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children but accept it only for pictures
    QGraphicsScene::dragEnterEvent(event);
    event->ignore();

    // skip bad mimes
    if (!event->mimeData() || !event->mimeData()->hasUrls())
        return;

    // get supported images extensions
    QStringList extensions;
    foreach (const QByteArray & format, QImageReader::supportedImageFormats())
        extensions.append( "." + format );

    // match each picture with urls
    foreach (const QUrl & url, event->mimeData()->urls()) {
        QString localFile = url.toLocalFile();
        foreach (const QString & extension, extensions) {
            if (localFile.endsWith(extension, Qt::CaseInsensitive)) {
                event->accept();
                return;
            }
        }
    }
}

void Desk::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children
    event->ignore();
    QGraphicsScene::dragMoveEvent(event);

    // or accept event for the Desk
    if (!event->isAccepted()) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void Desk::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    // dispatch to children
    event->ignore();
    QGraphicsScene::dropEvent(event);
    if (event->isAccepted())
        return;

    // or handle as a Desk drop event
    event->accept();
    double delta = 0;
    foreach (const QUrl & url, event->mimeData()->urls()) {
        QString localFile = url.toLocalFile();
        if (!QFile::exists(localFile))
            continue;

        // HACK: unify the loading code (3)
        PictureItem * p = new PictureItem(this);
        //p->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        connect(p, SIGNAL(configureMe()), this, SLOT(slotConfigurePicture()));
        connect(p, SIGNAL(backgroundMe()), this, SLOT(slotBackgroundPicture()));
        connect(p, SIGNAL(raiseMe()), this, SLOT(slotRaisePicture()));
        connect(p, SIGNAL(deleteMe()), this, SLOT(slotDeletePicture()));
        p->setPos(event->scenePos() + QPointF(delta, delta) );
        p->setZValue(++zLevel);
        if (!p->loadPhoto(localFile, true, true)) {
            delete p;
            continue;
        }
        p->show();
        m_pictures.append(p);
        delta += 30;
    }
}

void Desk::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    // first dispatch doubleclick to items
    mouseEvent->ignore();
    QGraphicsScene::mouseDoubleClickEvent(mouseEvent);
    if (mouseEvent->isAccepted())
        return;

    // unset the background picture, if present
    if (m_backPicture) {
        m_backPicture->show();
        m_backPicture = 0;
        m_backCache = QPixmap();
        update();
    }
}

/// Scene Background & Foreground
void Desk::drawBackground(QPainter * painter, const QRectF & rect)
{
    // draw picture if requested
    if (m_backPicture) {
        // regenerate cache if needed
        QSize sceneSize = sceneRect().size().toSize();
        if (m_backCache.isNull() || m_backCache.size() != sceneSize)
            m_backCache = m_backPicture->renderPhoto(sceneSize);

        // paint cached background
        QRect targetRect = rect.toRect();
        painter->drawPixmap(targetRect, m_backCache, targetRect);
        return;
    }

    // draw background
    QLinearGradient lg(m_rect.topLeft(), m_rect.bottomLeft());
    lg.setColorAt(0.0, m_grad1ColorPicker->color());
    lg.setColorAt(1.0, m_grad2ColorPicker->color());
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void Desk::drawForeground(QPainter * painter, const QRectF & /*rect*/)
{
    // draw header/footer
    QColor hColor = m_foreColorPicker->color();
    hColor.setAlpha(128);
    painter->fillRect(0, 0, m_size.width(), 50, hColor);
    painter->fillRect(0, m_size.height() - 50, m_size.width(), 50, hColor);

    // draw text
    painter->setFont(QFont("Courier 10 Pitch", 28));
    QLinearGradient lg(0,15,0,35);
/*  lg.setColorAt(0.0, Qt::white);
    lg.setColorAt(0.49, Qt::white);
    lg.setColorAt(0.51, m_titleColorPicker->color());
    lg.setColorAt(1.0, Qt::white); */
    QColor titleColor = m_titleColorPicker->color();
    lg.setColorAt(0.0, titleColor);
    lg.setColorAt(0.49, titleColor.lighter(150));
    lg.setColorAt(0.51, titleColor.darker(150));
    lg.setColorAt(1.0, titleColor);
    painter->setPen(QPen(lg, 0));
    painter->drawText(QRect(0, 0, m_size.width(), 50), Qt::AlignCenter, m_titleText);
}


/// Title
QString Desk::titleText() const
{
    return m_titleText;
}

void Desk::setTitleText(const QString & text)
{
    m_titleText = text;
    update(0, 0, m_size.width(), 50);
}


/// Slots
void Desk::slotConfigurePicture(const QPoint & scenePoint)
{
    PictureItem * picture = dynamic_cast<PictureItem *>(sender());
    if (!picture)
        return;

    // skip if an item is already present
    foreach (PicturePropertiesItem * item, m_properties)
        if (item->pictureItem() == picture)
            return;

    // create the properties item
    PicturePropertiesItem * pp = new PicturePropertiesItem(picture);
    connect(pp, SIGNAL(closed()), this, SLOT(slotDeleteProperties()));
    addItem(pp);
    pp->show();
    pp->setPos(scenePoint - QPoint(10, 10));
    pp->keepInBoundaries(sceneRect().toRect());

    // add to the internal list
    m_properties.append(pp);
}

void Desk::slotBackgroundPicture()
{
    PictureItem * picture = dynamic_cast<PictureItem *>(sender());
    if (!picture)
        return;

    // re-show previous background
    if (m_backPicture)
        m_backPicture->show();

    // hide current background picture
    m_backPicture = picture;
    m_backPicture->hide();
    m_backCache = QPixmap();
    update();
}

void Desk::slotRaisePicture()
{
    PictureItem * picture = dynamic_cast<PictureItem *>(sender());
    if (picture)
        picture->setZValue(++zLevel);
}

void Desk::slotDeletePicture()
{
    PictureItem * picture = dynamic_cast<PictureItem *>(sender());
    if (!picture)
        return;

    // unset background if deleting its picture
    if (m_backPicture == picture) {
        m_backPicture = 0;
        m_backCache = QPixmap();
        update();
    }

    // remove property if deleting its picture
    QList<PicturePropertiesItem *>::iterator ppIt = m_properties.begin();
    while (ppIt != m_properties.end()) {
        PicturePropertiesItem * pp = *ppIt;
        if (pp->pictureItem() == picture) {
            delete pp;
            ppIt = m_properties.erase(ppIt);
        } else
            ++ppIt;
    }

    // unlink picture from lists, myself(the Scene) and memory
    m_pictures.removeAll(picture);
    removeItem(picture);
    picture->deleteLater();
}

void Desk::slotDeleteProperties()
{
    PicturePropertiesItem * properties = dynamic_cast<PicturePropertiesItem *>(sender());
    if (!properties)
        return;

    // unlink picture from lists, myself(the Scene) and memory
    m_properties.removeAll(properties);
    removeItem(properties);
    properties->deleteLater();
}

void Desk::slotTitleColorChanged()
{
    update(0, 0, m_size.width(), 50);
}

void Desk::slotForeColorChanged()
{
    update(0, 0, m_size.width(), 50);
    update(0, m_size.height() - 50, m_size.width(), 50);
}

void Desk::slotGradColorChanged()
{
    update();
}
