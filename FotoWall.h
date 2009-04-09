/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
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
#include <QGraphicsView>
#include "ui_FotoWall.h"
class Desk;
class QNetworkReply;

class FotoWall : public QWidget
{
    Q_OBJECT
    public:
        FotoWall(QWidget * parent = 0);
        ~FotoWall();

        void showHelp();
        void checkForTutorial();
        void saveImage();
        void saveCD();
        void saveDVD();

    private:
        Ui::FotoWall *  ui;
        QGraphicsView * m_view;
        Desk *          m_desk;

    private Q_SLOTS:
        void on_projectType_currentIndexChanged(int index);
        void on_addPictures_clicked();
        void on_addText_clicked();
        void on_helpLabel_linkActivated(const QString & link);
        void on_tutorialLabel_linkActivated(const QString & link);
        void on_loadButton_clicked();
        void on_saveButton_clicked();
        void on_exportButton_clicked();
        void on_quitButton_clicked();
        void slotCheckTutorial(QNetworkReply * reply);
};

#endif
