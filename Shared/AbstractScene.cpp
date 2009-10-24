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

#include "AbstractScene.h"

AbstractScene::AbstractScene(QObject * parent)
  : QGraphicsScene(parent)
{
}

void AbstractScene::adjustSceneSize()
{
    emit geometryChanged();
}

void AbstractScene::resize(const QSize & size)
{
    // skip if already ok
    if (size == m_size)
        return;

    // save size and the related rect
    QSize oldSize = m_size;
    m_size = size;
    m_rect = QRectF(0, 0, m_size.width(), m_size.height());

    // call handlers
    //QResizeEvent re(m_size, oldSize);
    resizeEvent(/*&re*/);

    // change my rect
    setSceneRect(m_rect);
}

void AbstractScene::resizeEvent()
{
    // nothing to do here
}

