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

#include "FWScene.h"
#include "FWFoto.h"
#include "frames/PlasmaFrame.h"
#include "frames/HeartFrame.h"
#include "ColorPickerItem.h"
#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QList>
#include <QFile>

static int zLevel = 0;

#define COLORPICKER_W 200
#define COLORPICKER_H 150

FWScene::FWScene(QObject * parent)
    : QGraphicsScene(parent)
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

FWScene::~FWScene()
{
    delete m_titleColorPicker;
    delete m_foreColorPicker;
    delete m_grad1ColorPicker;
    delete m_grad2ColorPicker;
    qDeleteAll(m_photos);
    m_photos.clear();
}


void FWScene::resize(const QSize & size)
{
    m_size = size;
    m_rect = QRectF(0, 0, m_size.width(), m_size.height());
    m_titleColorPicker->setPos((size.width() - COLORPICKER_W) / 2.0, 10);
    m_grad1ColorPicker->setPos(size.width() - COLORPICKER_W, 0);
    m_grad2ColorPicker->setPos(size.width() - COLORPICKER_W, size.height() - COLORPICKER_H);
    setSceneRect(m_rect);
}

void FWScene::save(QDataStream & data) const
{
    // save own data
    data << m_titleColorPicker->color();
    data << m_foreColorPicker->color();
    data << m_grad1ColorPicker->color();
    data << m_grad2ColorPicker->color();
    data << m_titleText;

    // save the photos
    data << m_photos.size();
    foreach (FWFoto * foto, m_photos)
        foto->save(data);
}

void FWScene::restore(QDataStream & data)
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

    // restore the photos
    qDeleteAll(m_photos);
    m_photos.clear();
    int photos = 0;
    data >> photos;
    for (int i = 0; i < photos; i++) {
        FWFoto * foto = new FWFoto();
        //foto->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        foto->setFrame(new PlasmaFrame(":/plasma-frames/1.svg"));
        connect(foto, SIGNAL(deletePressed()), this, SLOT(slotDeleteFoto()));
        addItem(foto);
        foto->restore(data);
        m_photos.append(foto);
    }

    update();
}


/// Drag & Drop pictures
void FWScene::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    if (event->mimeData() && event->mimeData()->hasUrls()) {
        foreach (QUrl url, event->mimeData()->urls()) {
            QString sUrl = url.toString();
            if (sUrl.contains(".png", Qt::CaseInsensitive) || sUrl.contains(".jpg", Qt::CaseInsensitive)) {
                event->accept();
                return;
            }
        }
    }
    event->ignore();
}

void FWScene::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();
}

void FWScene::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    event->accept();

    QStringList localFiles;
    foreach (QUrl url, event->mimeData()->urls()) {
        QString fileName = url.toLocalFile();
        if (QFile::exists(fileName))
            localFiles << fileName;
    }

    int count = localFiles.size();
    for (int i = 0; i < count; i++) {
        double delta = 30.0 * ((double)i - ((double)count - 1.0) / 2.0);
        FWFoto * foto = new FWFoto();
        //foto->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        foto->setFrame(new PlasmaFrame(":/plasma-frames/1.svg"));
        //foto->setFrame((qrand() % 2) ? new HeartFrame() : new StandardFrame());
        connect(foto, SIGNAL(deletePressed()), this, SLOT(slotDeleteFoto()));
        addItem(foto);
        foto->setPos(event->scenePos() + QPointF(delta, delta) );
        foto->setZValue(++zLevel);
        foto->loadPhoto(localFiles[i], true, true);
        foto->show();
        m_photos.append(foto);
    }
}


/// Scene Background & Foreground
void FWScene::drawBackground(QPainter * painter, const QRectF & rect)
{
    // draw background
    QLinearGradient lg(m_rect.topLeft(), m_rect.bottomLeft());
    lg.setColorAt(0.0, m_grad1ColorPicker->color());
    lg.setColorAt(1.0, m_grad2ColorPicker->color());
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(rect, lg);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void FWScene::drawForeground(QPainter * painter, const QRectF & /*rect*/)
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
QString FWScene::titleText() const
{
    return m_titleText;
}

void FWScene::setTitleText(const QString & text)
{
    m_titleText = text;
    update(0, 0, m_size.width(), 50);
}


/// Slots
void FWScene::slotDeleteFoto()
{
    FWFoto * foto = dynamic_cast<FWFoto *>(sender());
    if (!foto)
        return;

    m_photos.removeAll(foto);
    removeItem(foto);
    foto->deleteLater();
}

void FWScene::slotTitleColorChanged()
{
    update(0, 0, m_size.width(), 50);
}

void FWScene::slotForeColorChanged()
{
    update(0, 0, m_size.width(), 50);
    update(0, m_size.height() - 50, m_size.width(), 50);
}

void FWScene::slotGradColorChanged()
{
    update();
}
