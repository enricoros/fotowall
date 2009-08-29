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
#include "ModeInfo.h"
class Desk;
class FWGraphicsView;
class QActionGroup;
class QGraphicsView;
class QMenu;
class QNetworkReply;
namespace Ui { class FotoWall; }

class FotoWall : public QWidget
{
    Q_OBJECT
    public:
        FotoWall(QWidget * parent = 0);
        ~FotoWall();

        void setModeInfo(ModeInfo modeInfo);
        ModeInfo getModeInfo(); // Needed for saving
        void restoreMode(int mode);

        void loadXml(const QString & filePath);
        void saveXml(const QString & filePath) const;

        void showIntroduction();
        void loadImages(QStringList &imagesPath);

    private:
        QMenu * createArrangeMenu();
        QMenu * createBackgroundMenu();
        QMenu * createDecorationMenu();
        QMenu * createOnlineHelpMenu();
        void createMiscActions();
        void checkForTutorial();
        void checkForSupport();
        void setNormalProject();
        void setCDProject();
        void setDVDProject();
        void setExactSizeProject();

        Ui::FotoWall *  ui;
        Desk *          m_desk;
        ModeInfo        m_modeInfo;
        QAction *       m_aHelpTutorial;
        QAction *       m_aHelpSupport;
        QActionGroup *  m_gBackActions;

    private Q_SLOTS:
        void on_projectType_activated(int index);
        void on_aAddFlickr_toggled(bool on);
        void on_aAddPicture_triggered();
        void on_aAddText_triggered();
        void on_aAddWebcam_triggered();
        void on_accelBox_toggled(bool checked);
        void on_transpBox_toggled(bool checked);
        void on_introButton_clicked();
        void on_loadButton_clicked();
        void on_saveButton_clicked();
        void on_exportButton_clicked();
        //void on_quitButton_clicked();

        void slotActionSelectAll();

        void slotArrangeForceField(bool enabled);
        void slotDecoTopBar(bool checked);
        void slotDecoBottomBar(bool checked);
        void slotDecoSetTitle();
        void slotDecoClearTitle();
        void slotHelpBlog();
        void slotHelpSupport();
        void slotHelpTutorial();
        void slotHelpUpdates();
        void slotSetBackMode(QAction* action);

        void slotBackModeChanged();
        void slotShowPropertiesWidget(QWidget *);

        void slotVerifyTutorial(QNetworkReply * reply);
        void slotVerifySupport(/*const KnowledgeItemV1List & items*/);
        void slotVerifyVideoInputs(int count);
};

#endif
