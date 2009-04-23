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

#include <QGraphicsView>
#include <QWidget>
#include "ModeInfo.h"
class Desk;
class OcsConnector;
class QMenu;
class QNetworkReply;
namespace Ui { class FotoWall; }

class FotoWall : public QWidget
{
    Q_OBJECT
    public:
        FotoWall(QWidget * parent = 0);
        ~FotoWall();

        void showIntroduction();
        void checkForTutorial();
        void saveImage();
        void saveExactSize();
        void load(QString &fileName);
        ModeInfo getModeInfo(); // Needed for saving
        void restoreMode(int);
        void setModeInfo(ModeInfo);

    private:
        void createActions();
        QMenu * createDecorationMenu();
        Ui::FotoWall *  ui;
        QGraphicsView * m_view;
        Desk *          m_desk;
        OcsConnector *  m_ocsConnector;
        ModeInfo        m_modeInfo;

        void loadNormalProject();
        void loadCDProject();
        void loadDVDProject();
        void loadExactSizeProject();

    private Q_SLOTS:
        void on_projectType_currentIndexChanged(int index);
        void on_addPictures_clicked();
        void on_addText_clicked();
        void on_addMirror_clicked();
        void on_helpLabel_linkActivated(const QString & link);
        void on_tutorialLabel_linkActivated(const QString & link);
        void on_loadButton_clicked();
        void on_saveButton_clicked();
        void on_exportButton_clicked();
        void on_quitButton_clicked();

        void slotDecoTopBar(bool checked);
        void slotDecoBottomBar(bool checked);
        void slotDecoSetTitle();
        void slotDecoClearTitle();

        void slotActionSelectAll();

        void slotCheckTutorial(QNetworkReply * reply);
        void slotVideoInputsChanged(int count);
};

#endif
