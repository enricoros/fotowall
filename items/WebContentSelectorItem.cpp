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

#include "WebContentSelectorItem.h"
#include "frames/FrameFactory.h"
#include "frames/Frame.h"
#include "3rdparty/gsuggest.h"
#include "ui_WebContentSelectorItem.h"
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsLinearLayout>
#include <QListWidget>
#include <QPainter>

WebContentSelectorItem::WebContentSelectorItem(QGraphicsItem * parent)
    : QGraphicsWidget(parent)
    , m_frame(FrameFactory::createFrame(0x1001 /*HARDCODED*/))
    , m_completion(0)
    , m_ui(new Ui_WebContentSelectorItem())
{
    // create & customize widget
    QWidget * widget = new QWidget();
    widget->setAttribute(Qt::WA_TranslucentBackground);
    m_ui->setupUi(widget);
    QPalette pal;
    pal.setBrush(QPalette::Base, Qt::transparent);
    m_ui->listWidget->setPalette(pal);
    connect(m_ui->searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

    // embed and layout widget
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
    QGraphicsLinearLayout * vLay = new QGraphicsLinearLayout(Qt::Vertical, this);
    vLay->addItem(proxy);

    // apply google completion to widget
    m_completion = new GSuggestCompletion(m_ui->lineEdit);
}

WebContentSelectorItem::~WebContentSelectorItem()
{
    delete m_completion;
    delete m_frame;
    delete m_ui;
}

void WebContentSelectorItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // draw frame
    m_frame->paint(painter, boundingRect().toRect(), false, false);
}

void WebContentSelectorItem::doSearch()
{
    // get the current search term
    m_completion->preventSuggest();
    QString searchName = m_ui->lineEdit->text();
    if (searchName.isEmpty())
        return;

    // start a flickr search
    m_ui->listWidget->clear();
    // TODO
}
