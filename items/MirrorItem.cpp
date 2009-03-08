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

#include "MirrorItem.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <math.h>

#define MIRROR_HEIGHT 100

MirrorItem::MirrorItem(PictureItem * copyItem, QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_item(copyItem)
{
    // add to the scame scene
    m_item->scene()->addItem(this);

    // read current values
    m_boundingRect = m_item->boundingRect();
    setVisible(m_item->isVisible());
    slotGfxChange();

    // monitor changes
    connect(m_item, SIGNAL(gfxChange()), this, SLOT(slotGfxChange()));
    connect(m_item, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

MirrorItem::~MirrorItem()
{
}

QRectF MirrorItem::boundingRect() const
{
    return m_boundingRect;
}
#include <QDebug>
void MirrorItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * /*widget*/)
{
    // generate Reflection pixmap, if needed
    if (m_pixmap.isNull()) {

        // start with a blank pixmap
        m_pixmap = QPixmap(m_boundingRect.width(), m_boundingRect.height());
        m_pixmap.fill(Qt::transparent);

        // find out the Transform chain to mirror a rotated item
        QRectF itemRect = m_item->boundingRect();
        QPolygonF scenePolygon = m_item->mapToScene(itemRect);
        QRectF sceneRectF = scenePolygon.boundingRect();

#if 0
        QTransform itemTransform = m_item->transform();
        itemTransform.translate(m_item->pos().x(), m_item->pos().y());

        QTransform pixmapTransform = QTransform(1, 0, 0, -1, 0, 0);
        pixmapTransform.translate(sceneRectF.left(), sceneRectF.top());


//        QTransform pixmapTransform(1, 0, 0, -1, 0, 0);

        QTransform mirrorTransform = pixmapTransform.inverted() * itemTransform;
        qWarning() << itemTransform << pixmapTransform << sceneRectF.top();

//        mirrorTransform.translate(sceneRectF.width() / 2, sceneRectF.height() / 2);
////        mirrorTransform.translate(sceneRectF.width() / 2, sceneRectF.height() / 2);
//        mirrorTransform = m_item->transform().inverted();
//        qWarning() << m_item->transform() << mirrorTransform;
//        mirrorTransform.translate(-sceneRectF.width() / 2, -sceneRectF.height() / 2);

//        QTransform mirrorTransform;// = m_item->transform().inverted();

#else
        QTransform itemTransform = m_item->transform();
        QPointF xAxis = itemTransform.map(QPointF(1.0, 0.0));
        double zRotation = atan2(xAxis.y(), xAxis.x());

        QTransform mirrorTransform;
        mirrorTransform.translate(sceneRectF.width() / 2, sceneRectF.height() / 2);
        mirrorTransform.rotateRadians(-zRotation);
#endif

        QPainter p(&m_pixmap);
        p.setTransform(mirrorTransform);
        m_item->paint(&p, 0, 0);
        p.end();

        // add a linear alpha channel to the image
        QPixmap alphaPixmap(m_pixmap.size());
        QPainter alphaPainter(&alphaPixmap);
        QLinearGradient alphaGradient(0, 0, 0, alphaPixmap.height());
        alphaGradient.setColorAt(0.0 , QColor(128, 128, 128));
        alphaGradient.setColorAt(0.25, QColor( 32,  32,  32));
        alphaGradient.setColorAt(1.0 , QColor(  0,   0,   0));
        alphaPainter.fillRect(alphaPixmap.rect(), alphaGradient);
        alphaPainter.end();
        m_pixmap.setAlphaChannel(alphaPixmap);
    }

    // draw the reflection pixmap
    painter->drawPixmap(option->rect, m_pixmap, option->rect);
}

void MirrorItem::slotGfxChange() // PictureItem independant
{
    // find out the item's polygon in scene coordinates
    QRectF itemRect = m_item->boundingRect();
    QPolygonF itemScenePolygon = m_item->mapToScene(itemRect);

    // resize and reposition as an axis-aligned rect on the bottom of the Picture
    QRect sceneRect = itemScenePolygon.boundingRect().toRect();
#if 1
    setPos(sceneRect.bottomLeft());
#else
    setPos(sceneRect.bottomLeft() + QPoint(0, -5));
    setZValue(m_item->zValue() - 0.1);
#endif
    prepareGeometryChange();
    m_boundingRect = sceneRect.translated(-sceneRect.topLeft());
    if (m_boundingRect.height() > MIRROR_HEIGHT)
        m_boundingRect.setHeight(MIRROR_HEIGHT);

    // invalidate current rendering
    m_pixmap = QPixmap();
    update();
}
