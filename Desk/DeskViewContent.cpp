/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "DeskViewContent.h"

#include "Desk.h"
#include "App/XmlRead.h"

#include <QPainter>

DeskViewContent::DeskViewContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_innerDesk(0)
{
}

bool DeskViewContent::load(const QString & filePath, bool keepRatio, bool setName)
{
    if (filePath.isNull())
        return false;

    m_innerDesk = new Desk(this);
    m_innerDesk->resize(QSize(800, 600));

    XmlRead *xmlRead = 0;
    try {
        xmlRead = new XmlRead(filePath);
    } catch (...) {
        // If loading failed
        return false;
    }
    //xmlRead->readProject(this);
    xmlRead->readDesk(m_innerDesk);
    xmlRead->readContent(m_innerDesk);
    delete xmlRead;
    return true;
}

QWidget * DeskViewContent::createPropertyWidget()
{
    return 0;
}

bool DeskViewContent::fromXml(QDomElement & parentElement)
{
    return false;
}

void DeskViewContent::toXml(QDomElement & parentElement) const
{
}

QPixmap DeskViewContent::renderContent(const QSize & size, Qt::AspectRatioMode ratio) const
{
    return QPixmap(100, 100);
}

bool DeskViewContent::contentOpaque() const
{
    return false;
}

#include "App/MainWindow.h"
void DeskViewContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    m_innerDesk = MainWindow::instance()->swapDesk(m_innerDesk);
    update();
}

void DeskViewContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (m_innerDesk)
        m_innerDesk->render(painter, boundingRect(), m_innerDesk->sceneRect(), Qt::IgnoreAspectRatio);
    else
        painter->fillRect(boundingRect(), Qt::red);
}
