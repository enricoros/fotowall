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
        void saveImage();
        void saveExactSize();
        void load(QString &fileName);
        ModeInfo getModeInfo(); // Needed for saving
        void restoreMode(int);
        void setModeInfo(ModeInfo);

    private:
        QMenu * createArrangeMenu();
        QMenu * createBackgroundMenu();
        QMenu * createDecorationMenu();
        QMenu * createHelpMenu();
        void createMiscActions();
        void checkForTutorial();
        void checkForSupport();
        void loadNormalProject();
        void loadCDProject();
        void loadDVDProject();
        void loadExactSizeProject();
        QImage renderedImage(const QSize &size);

        Ui::FotoWall *  ui;
        QGraphicsView * m_view;
        Desk *          m_desk;
        ModeInfo        m_modeInfo;
        QAction *       m_aHelpTutorial;
        QAction *       m_aHelpSupport;

    private Q_SLOTS:
        void on_projectType_currentIndexChanged(int index);
        void on_addPictures_clicked();
        void on_addText_clicked();
        void on_addMirror_clicked();
        void on_loadButton_clicked();
        void on_saveButton_clicked();
        void on_exportButton_clicked();
        void on_quitButton_clicked();

        void slotActionSelectAll();

        void slotArrangeForceField(bool enabled);
        void slotBackGradient(bool checked);
        void slotDecoTopBar(bool checked);
        void slotDecoBottomBar(bool checked);
        void slotDecoSetTitle();
        void slotDecoClearTitle();
        void slotHelpIntroduction();
        void slotHelpTutorial();
        void slotHelpSupport();

        void slotVerifyTutorial(QNetworkReply * reply);
        void slotVerifySupport(/*const KnowledgeItemV1List & items*/);
        void slotVerifyVideoInputs(int count);
};

#endif
