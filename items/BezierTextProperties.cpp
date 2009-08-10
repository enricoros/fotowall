/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Tanguy Arnaud <arn.tanguy@gmail.com             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "BezierTextProperties.h"
#include "BezierTextContent.h"
#include "3rdparty/bezier/bezier.h"

BezierTextProperties::BezierTextProperties(BezierTextContent * bezierTextContent, QGraphicsItem * parent)
    : AbstractProperties(bezierTextContent, parent)
      , m_bezierTextContent (bezierTextContent)
{
    m_bezierWidget = new Bezier();
    m_bezierWidget->setFont(m_bezierTextContent->font());
    m_bezierWidget->setFontSize(m_bezierTextContent->font().pointSize());
    m_bezierWidget->setText(m_bezierTextContent->text());
    m_bezierWidget->setControlPoints(m_bezierTextContent->controlPoints());
    addTab(m_bezierWidget, tr("Bezier Text"), false, true);
}

BezierTextProperties::~BezierTextProperties()
{
}

void BezierTextProperties::closing()
{
    m_bezierTextContent->setText(m_bezierWidget->text());
    m_bezierTextContent->setPath(m_bezierWidget->path());
    m_bezierTextContent->setFont(m_bezierWidget->font());
    m_bezierTextContent->setFontSize(m_bezierWidget->fontSize());
    m_bezierTextContent->setControlPoints(m_bezierWidget->controlPoints());
}
