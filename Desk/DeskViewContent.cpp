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

#include "App/XmlRead.h"
#include "Desk.h"

#include <QFileInfo>
#include <QPainter>

DeskViewContent::DeskViewContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_desk(0)
{
}

bool DeskViewContent::load(const QString & filePath, bool keepRatio, bool setName)
{
    // parse the DOM of the file
    XmlRead xmlRead;
    if (!xmlRead.loadFile(filePath))
        return false;

    setFrameTextEnabled(setName);
    setFrameText(QFileInfo(filePath).baseName());

    // create the new Desk
    m_desk = new Desk(this);
    connect(m_desk, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintDesk(const QList<QRectF> &)));
    m_desk->resize(QSize(800, 600));

    // read in the properties
    //xmlRead.readProject(this);
    xmlRead.readDesk(m_desk);
    xmlRead.readContent(m_desk);
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

#include "App/App.h"
#include "App/MainWindow.h"
void DeskViewContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event)
{
    App::mainWindow->stackDesk(m_desk);
    update();
}

void DeskViewContent::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    // TODO: see if it paints when the scene is not displayed...
    if (m_desk)
        m_desk->render(painter, boundingRect(), m_desk->sceneRect(), Qt::IgnoreAspectRatio);
    else
        painter->fillRect(boundingRect(), Qt::red);
}

void DeskViewContent::slotRepaintDesk(const QList<QRectF> & /*exposed*/)
{
    update();
}
