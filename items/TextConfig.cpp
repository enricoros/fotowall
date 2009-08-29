/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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
#include "TextContent.h"

TextConfig::TextConfig(TextContent * textContent, QGraphicsItem * parent)
    : AbstractConfig(textContent, parent)
    , m_textContent(textContent)
{
    // inject Text Editor
    m_editor = new RichTextEditorDialog();
    m_editor->setMinimumSize(320, 200);
    m_editor->setText(m_textContent->toHtml());
    m_editor->adjustSize();
    addTab(m_editor, tr("Text"), false, true);
    m_editor->focusEditor();

    // show the ok button
    showOkButton(true);
}

TextConfig::~TextConfig()
{
}

void TextConfig::slotOkClicked()
{
    m_textContent->setHtml(m_editor->text(Qt::RichText));
    slotRequestClose();
}
