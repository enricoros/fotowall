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

#include "TextProperties.h"
#include "TextContent.h"
#include "richtexteditor_p.h"
#include <QSettings>

TextProperties::TextProperties(TextContent * textContent, QGraphicsItem * parent)
    : AbstractProperties(textContent, parent)
    , m_textContent(textContent)
{
    // Inject Editor setup
    m_editor = new RichTextEditorDialog();
    m_editor->setMinimumSize(450, 200);
    m_editor->setText(m_textContent->toHtml());
    addTab(m_editor, tr("Text"), false, true);
}

TextProperties::~TextProperties()
{
    m_textContent->setHtml(m_editor->text(Qt::RichText));
}

