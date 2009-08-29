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

#ifndef __TextProperties_h__
#define __TextProperties_h__

#include <QWidget>
#include "ui_TextProperties.h"

class TextProperties : public QWidget, public Ui::TextProperties {
    Q_OBJECT
    public:
        friend class TextContent;
        TextProperties(QWidget *parent = 0);
        ~TextProperties();
};

#endif
