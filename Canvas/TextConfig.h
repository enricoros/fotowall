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

#ifndef __TextConfig_h__
#define __TextConfig_h__

#include "AbstractConfig.h"
class RichTextEditorDialog;
class TextContent;

class TextConfig : public AbstractConfig
{
    Q_OBJECT
    public:
#if !defined(MOBILE_UI)
        TextConfig(TextContent * textContent, QGraphicsItem * parent = 0);
#else
        TextConfig(TextContent * textContent, QWidget * parent = 0);
#endif

    private:
        // ::AbstractConfig
        void slotOkClicked();
        TextContent * m_textContent;
        RichTextEditorDialog * m_editor;
};

#endif
