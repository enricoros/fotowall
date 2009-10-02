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

#include "CanvasViewContent.h"

#include "App/XmlRead.h"
#include "Canvas.h"

#include <QFileInfo>
#include <QPainter>

CanvasViewContent::CanvasViewContent(QGraphicsScene * scene, QGraphicsItem * parent)
    : AbstractContent(scene, parent, false)
    , m_canvas(0)
{
}

bool CanvasViewContent::load(const QString & filePath, bool /*keepRatio*/, bool setName)
{
    // parse the DOM of the file
    XmlRead xmlRead;
    if (!xmlRead.loadFile(filePath))
        return false;

    setFrameTextEnabled(setName);
    setFrameText(QFileInfo(filePath).baseName());

    // create the new Canvas
    m_canvas = new Canvas(this);
    connect(m_canvas, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(slotRepaintCanvas(const QList<QRectF> &)));
    m_canvas->resize(QSize(800, 600));

    // read in the properties
    //xmlRead.readProject(this);
    xmlRead.readCanvas(m_canvas);
    xmlRead.readContent(m_canvas);
    return true;
}

QWidget * CanvasViewContent::createPropertyWidget()
{
    return 0;
}

bool CanvasViewContent::fromXml(QDomElement & /*parentElement*/)
{
    return false;
}

void CanvasViewContent::toXml(QDomElement & /*parentElement*/) const
{
}

void CanvasViewContent::drawContent(QPainter * painter, const QRect & targetRect)
{
    // TODO: see if it paints when the scene is not displayed...
    if (m_canvas)
        m_canvas->render(painter, targetRect, m_canvas->sceneRect(), Qt::IgnoreAspectRatio);
    else
        painter->fillRect(targetRect, Qt::red);
}

QPixmap CanvasViewContent::renderContent(const QSize & /*size*/, Qt::AspectRatioMode /*ratio*/) const
{
    return QPixmap(100, 100);
}

bool CanvasViewContent::contentOpaque() const
{
    return false;
}

#include "App/App.h"
#include "App/MainWindow.h"
void CanvasViewContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{
    App::mainWindow->editCanvas(m_canvas);
}

void CanvasViewContent::slotRepaintCanvas(const QList<QRectF> & /*exposed*/)
{
    update();
}
