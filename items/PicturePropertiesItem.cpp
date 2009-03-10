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
#include "frames/FrameFactory.h"
#include <QGraphicsSceneMouseEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QWidget>

PicturePropertiesItem::PicturePropertiesItem(PictureItem * pictureItem, QGraphicsItem * parent)
    : QGraphicsProxyWidget(parent)
    , m_ui(new Ui::PicturePropertiesItem())
    , m_pictureItem(0)
    , m_frame(FrameFactory::defaultFrame())
    , m_aniStep(0)
    , m_aniDirection(true)
{
    // WIDGET setup
    QWidget * widget = new QWidget();
    widget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_ui->setupUi(widget);

    // add frame items to the listview
    foreach (quint32 frameClass, FrameFactory::classes()) {
        // make icon from frame preview
        Frame * frame = FrameFactory::createFrame(frameClass);
        QIcon icon(frame->preview(48, 48));
        delete frame;

        // add the item to the list (and attach it the class)
        QListWidgetItem * item = new QListWidgetItem(icon, QString(), m_ui->listWidget);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        item->setData(Qt::UserRole, frameClass);
    }

    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(slotClickedOk()));
    connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(slotClickedCancel()));
    connect(m_ui->listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotFrameSelected(QListWidgetItem*)));

    // load values
    if (pictureItem)
        setPictureItem(pictureItem);

    // ITEM setup
    setWidget(widget);
    setZValue(99999);

    // Transition setup
    m_aniTimer.start(20, this);
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

    // select the frame
    quint32 frameClass = m_pictureItem->frameClass();
    for (int i = 0; i < m_ui->listWidget->count(); ++i) {
        QListWidgetItem * item = m_ui->listWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == frameClass) {
            item->setSelected(true);
            break;
        }
    }

}

void PicturePropertiesItem::applyProperties()
{
    qWarning("NOT IMPLEMENTED 2");
}

void PicturePropertiesItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::RightButton)
        slotClickedOk();
    QGraphicsProxyWidget::mousePressEvent(event);
}

void PicturePropertiesItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if (m_aniStep < 10)
        return;

    // draw custom background
    m_frame->paint(painter, boundingRect().toRect(), false);

    // unbreak parent
    if (m_aniStep >= 10 && m_aniStep <= 90)
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    QGraphicsProxyWidget::paint(painter, option, widget);
}

void PicturePropertiesItem::timerEvent(QTimerEvent * event)
{
    // only act on our events
    if (event->timerId() == m_aniTimer.timerId()) {
        if (m_aniDirection) {
            m_aniStep += 5;
            if (m_aniStep >= 100) {
                m_aniStep = 100;
                resetTransform();
                m_aniTimer.stop();
            } else {
                qreal xCenter = boundingRect().center().x();
                setTransform(QTransform().translate(xCenter, 0).rotate((90*(100-m_aniStep)*(100-m_aniStep)) / 10000, Qt::XAxis).translate(-xCenter, 0));
            }
        } else {
            m_aniStep -= 5;
            if (m_aniStep <= 0) {
                m_aniStep = 0;
                resetTransform();
                m_aniTimer.stop();
                // CHECK if this is the right place
                hide();
            } else {
                qreal xCenter = boundingRect().center().x();
                setTransform(QTransform().translate(xCenter, 0).rotate(-90 + (90*m_aniStep*m_aniStep) / 10000, Qt::XAxis).translate(-xCenter, 0));
            }
        }
    }
    QObject::timerEvent(event);
}

void PicturePropertiesItem::slotFrameSelected(QListWidgetItem * item)
{
    // get the frame class
    if (!item)
        return;
    quint32 frameClass = item->data(Qt::UserRole).toUInt();
    if (!frameClass)
        return;

    // create and set the frame
    Frame * frame = FrameFactory::createFrame(frameClass);
    if (frame)
        m_pictureItem->setFrame(frame);
}

void PicturePropertiesItem::slotClickedOk()
{
    // apply and closure animation
    applyProperties();
    m_aniDirection = false;
    m_aniTimer.start(20, this);
}

void PicturePropertiesItem::slotClickedCancel()
{
    // closure animation only
    m_aniDirection = false;
    m_aniTimer.start(20, this);
}

