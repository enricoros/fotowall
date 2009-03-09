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

#include "PicturePropertiesItem.h"
#include "ui_PicturePropertiesItem.h"
#include "frames/PlasmaFrame.h"
#include <QWidget>
#include <QPainter>

PicturePropertiesItem::PicturePropertiesItem(PictureItem * pictureItem, QGraphicsItem * parent)
    : QGraphicsProxyWidget(parent)
    , m_ui(new Ui::PicturePropertiesItem())
    , m_pictureItem(0)
    , m_frame(new PlasmaFrame())
{
    // setup the gfx inside the widget
    QWidget * widget = new QWidget();
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_ui->setupUi(widget);

    // load values
    if (pictureItem)
        setPictureItem(pictureItem);

    // set the widget as our content
    setWidget(widget);
    setZValue(99999);
}

PicturePropertiesItem::~PicturePropertiesItem()
{
    delete m_frame;
    delete m_ui;
}

void PicturePropertiesItem::setPictureItem(PictureItem * pictureItem)
{
    m_pictureItem = pictureItem;
    loadProperties();
}

PictureItem * PicturePropertiesItem::pictureItem() const
{
    return m_pictureItem;
}

void PicturePropertiesItem::loadProperties()
{
    qWarning("NOT IMPLEMENTED 1");
}

void PicturePropertiesItem::applyProperties()
{
    qWarning("NOT IMPLEMENTED 2");
}

void PicturePropertiesItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    // draw custom background
    m_frame->paint(painter, boundingRect().toRect(), false);

    // unbreak parent
    QGraphicsProxyWidget::paint(painter, option, widget);
}

void PicturePropertiesItem::on_buttonBox_accepted()
{
    // apply and close
    applyProperties();
    hide();
}

void PicturePropertiesItem::on_buttonBox_rejected()
{
    // close only
    hide();
}
