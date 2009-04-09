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

#ifndef __TextProperties_h__
#define __TextProperties_h__

#include "AbstractProperties.h"
class QListWidgetItem;
class RichTextEditorDialog;
class TextContent;


class TextProperties : public AbstractProperties {
    Q_OBJECT
    public:
        TextProperties(TextContent * textContent, QGraphicsItem * parent = 0);
        ~TextProperties();

    private:
        TextContent * m_textContent;
        RichTextEditorDialog * m_editor;

    private Q_SLOTS:
        void slotClosing();
};

#endif
