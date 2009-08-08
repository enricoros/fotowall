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
#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QString>
#include "frames/FrameFactory.h"

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

HelpItem::HelpItem(QGraphicsItem * parent)
    : AbstractDisposeable(parent, true)
    , m_frame(FrameFactory::createFrame(0x1001 /*HARDCODED*/))
{
    QString introduction = tr("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
"<html><!--head><meta name=\"qrichtext\" content=\"1\" /></head-->\n"
"<body style=\"font-family:'Sans Serif';\">\n"
"  <div>\n"
"    <img src=':/data/action-remove.png'>\n"
"    <span style='font-size: 26px; font-weight: normal; color: #FF0000;'>Welcome</span>\n"
"    <img src=':/data/action-remove.png'>\n"
"  </div>\n"
"\n"
"  <br />\n"
"  <div style='font-size: 16px; font-weight: normal;'><b>Playing</b> with <b>FotoWall</b> is easy</div>\n"
"\n"
"  <div>\n"
"    <ul>\n"
"      <li><b><img src=':/data/action-add.png'>&nbsp;&nbsp;&nbsp;add</b> a picture&nbsp;using the&nbsp;<i><span style='background-color: rgb(255, 226, 0);'>add pictures</span></i>&nbsp;button or <i><span style='background-color: rgb(255, 226, 0);'>drop it</span></i></li>\n"
"      <li><i><img src=':/data/action-rotate.png'>&nbsp;&nbsp; to <b>place it where you want</b>,&nbsp;<span style='background-color: rgb(255, 226, 0);'>try</span> <span style='background-color: rgb(255, 226, 0);'><i>every small icon</i></span><i>&nbsp;</i>on the frame</i></li>\n"
"      <li><i><img src=':/data/action-configure.png'>&nbsp;&nbsp;&nbsp;<span style='background-color: rgb(255, 226, 0);'>right click</span></i> <span style='background-color: rgb(255, 226, 0);'>on the picture</span> to <b>change its look</b></li>\n"
"      <li><i><span style='background-color: rgb(255, 226, 0);'>double click</span></i><b><span style='background-color: rgb(255, 226, 0);'>&nbsp;</span></b><span style='background-color: rgb(255, 226, 0);'>the picture</span> to set it as <b>background</b></li>\n"
"      <li><i><span style='background-color: rgb(255, 226, 0);'>click</span></i><span style='background-color: rgb(255, 226, 0);'>&nbsp;the&nbsp;corners</span> to change wall&nbsp;<b>colors</b></li>\n"
"      <li>just look and you will find ...</li>\n"
"    </ul>\n"
"  </div>\n"
"\n"
"  <div><font color='#2B2B2B'><b>Please make sure to share your best work with your friends ;-)</b></font></div>\n"
"\n"
"  <br/>\n"
"  <div><font color='#616161' style='font-size: 11px; font-weight: normal;'>FotoWall is LGPL, by Enrico Ros &lt;enrico.ros@gmail.com&gt;, 2009</font></div>\n"
"</body>\n"
"</html>");
#if defined(USE_QTWEBKIT)
    // show fancy help in internal browser
#error WebKit was completely disabled, to only have 1 version of the HTML to translate...
    //BrowserItem * bi = new BrowserItem(this);
    //bi->setGeometry(m_frame->contentsRect(boundingRect().toRect()));
    //bi->write(introduction);
    //bi->setReadOnly(true);
#else
    // create an item to display it
    QGraphicsTextItem * ti = new QGraphicsTextItem(this);
    ti->setHtml(introduction);
    ti->setPos(boundingRect().adjusted(12, 12, -12, -12).topLeft());
    ti->setTextInteractionFlags(Qt::NoTextInteraction);
#endif
}

HelpItem::~HelpItem()
{
    delete m_frame;
}

QRectF HelpItem::boundingRect() const
{
    QRectF r = childrenBoundingRect ().adjusted(-12, -12, 12, 12);
    r.moveCenter(QPointF());
    return r;
}

void HelpItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    m_frame->paint(painter, boundingRect().toRect(), false, false);
}

void HelpItem::mousePressEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    emit closeMe();
}
