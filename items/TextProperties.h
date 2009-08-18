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

#include <QWidget>

namespace Ui { class TextProperties; }

class TextProperties : public QWidget {
    Q_OBJECT
    public:
        TextProperties(QWidget *parent = 0);
        ~TextProperties();

    private:
        Ui::TextProperties *ui;
};

#endif
