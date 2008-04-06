/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FotoWall_h__
#define __FotoWall_h__

#include <QWidget>
#include <QLabel>

#include "ui_FotoWall.h"
#include <QGraphicsView>
class FWScene;

class FotoWall : public QWidget, public Ui::FotoWall
{
    Q_OBJECT
    public:
        FotoWall(QWidget * parent = 0);
        ~FotoWall();

    private:
        QGraphicsView * m_view;
        FWScene *       m_scene;

    private Q_SLOTS:
        void on_loadButton_clicked();
        void on_saveButton_clicked();
        void on_pngButton_clicked();
        void on_quitButton_clicked();
        void on_titleEdit_textChanged( const QString & text );
};

#endif
