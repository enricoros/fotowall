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

#include "TextConfig.h"
#include "3rdparty/richtextedit/richtexteditor_p.h"
#include "3rdparty/bezier/bezier.h"
#include "TextContent.h"

TextConfig::TextConfig(TextContent * textContent, QGraphicsItem * parent)
    : AbstractConfig(textContent, parent)
    , m_textContent(textContent)
{
    // inject Text Editor
    m_editor = new RichTextEditorDialog();
    m_editor->setMinimumSize(430, 200);
    m_editor->setText(m_textContent->toHtml());
    m_editor->adjustSize();
    addTab(m_editor, tr("Text"), false, true);

    // inject Bezier Shape Editor
    m_bezierWidget = new Bezier();
    m_bezierWidget->setEnabled(m_textContent->shapeEnabled());
    m_bezierWidget->setFont(m_textContent->defaultFont());
    m_bezierWidget->setText(m_textContent->toPlainText());
    m_bezierWidget->setControlPoints(m_textContent->shapeControlPoints());
    addTab(m_bezierWidget, tr("Bezier Path"), false, false);

    m_editor->focusEditor();
}

TextConfig::~TextConfig()
{
}

void TextConfig::closing()
{
    m_textContent->setHtml(m_editor->text(Qt::RichText));
    m_textContent->setShapeEnabled(m_bezierWidget->enabled());
    m_textContent->setShapeControlPoints(m_bezierWidget->controlPoints());
}
