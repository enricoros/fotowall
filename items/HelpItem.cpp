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

#include "HelpItem.h"
#if defined(USE_QTWEBKIT)
#include "BrowserItem.h"
#else
#include <QGraphicsTextItem>
#endif
#include <QFile>
#include <QLocale>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "frames/FrameFactory.h"

/// return the translated intro file, or fall back to english
static QString translatedIntro(const QString & extension)
{
    // return the translated file name...
    QString langCode = QLocale::system().name().split('_').first();
    QString fileName = QString(":/translations/introduction-%1.%2").arg(langCode, extension);
    if (QFile::exists(fileName))
        return fileName;

    // ...or fall back to english
    return QString(":/translations/introduction-en.%1").arg(extension);
}

HelpItem::HelpItem(QGraphicsItem * parent)
    : QGraphicsItem(parent)
    , m_frame(FrameFactory::createFrame(0x1001 /*HARDCODED*/))
{
#if defined(USE_QTWEBKIT)
    // show fancy help in internal browser
#error WebKit was completely disabled, to only have 1 version of the HTML to translate...
    //BrowserItem * bi = new BrowserItem(this);
    //bi->setGeometry(m_frame->contentsRect(boundingRect().toRect()));
    //bi->browse("qrc" + translatedIntro("html")); // qrc:/translations/introduction-LANG.html
    //bi->setReadOnly(true);
#else
    // get html text
    QFile htmlFile(translatedIntro("html"));
    htmlFile.open(QIODevice::ReadOnly);

    // create an item to display it
    QGraphicsTextItem * ti = new QGraphicsTextItem(this);
    ti->setPos(10, 10/*FIXME m_frame->contentsRect(boundingRect().toRect()).topLeft()*/);
    ti->setHtml(htmlFile.readAll());
    ti->setTextInteractionFlags(Qt::NoTextInteraction);
#endif
}

HelpItem::~HelpItem()
{
    delete m_frame;
}

QRectF HelpItem::boundingRect() const
{
    return QRectF(-280, -160, 560, 320);
}

void HelpItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    m_frame->paint(painter, boundingRect().toRect(), false, false);
}

void HelpItem::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    emit closeMe();
}
