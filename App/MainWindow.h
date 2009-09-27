/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __MainWindow_h__
#define __MainWindow_h__

#include <QWidget>
#include "ModeInfo.h"
class Canvas;
class LikeBack;
class QActionGroup;
class QGraphicsView;
class QMenu;
class QNetworkReply;
namespace Ui { class MainWindow; }

class MainWindow : public QWidget
{
    Q_OBJECT
    public:
        MainWindow(const QStringList & loadUrls = QStringList(), QWidget * parent = 0);
        ~MainWindow();

        // TEMP
        void stackCanvas(Canvas * newCanvas);

        void setModeInfo(ModeInfo modeInfo);
        ModeInfo getModeInfo(); // Needed for saving
        void restoreMode(int mode);

        void showIntroduction();

        // ::QWidget
        void closeEvent(QCloseEvent * event);

    private:
        QMenu * createArrangeMenu();
        QMenu * createBackgroundMenu();
        QMenu * createDecorationMenu();
        QMenu * createOnlineHelpMenu();
        void createLikeBack();
        void createMiscActions();
        void checkForTutorial();
        void checkForSupport();
        void checkForUpdates();
        void setNormalProject();
        void setCDProject();
        void setDVDProject();
        void setExactSizeProject();

        Ui::MainWindow * ui;
        Canvas *        m_canvas;
        ModeInfo        m_modeInfo;
        QAction *       m_aHelpTutorial;
        QAction *       m_aHelpSupport;
        QActionGroup *  m_gBackActions;
        QActionGroup *  m_gBackRatioActions;
        QString         m_website;
        LikeBack *      m_likeBack;

    private Q_SLOTS:
        void on_projectType_activated(int index);
        void on_aAddCanvas_triggered();
        void on_aAddFlickr_toggled(bool on);
        void on_aAddPicture_triggered();
        void on_aAddText_triggered();
        void on_aAddWebcam_triggered();
        void on_accelBox_toggled(bool checked);
        void on_transpBox_toggled(bool checked);
        void on_introButton_clicked();
        void on_lbBug_clicked();
        void on_lbFeature_clicked();
        void on_lbDislike_clicked();
        void on_lbLike_clicked();
        bool on_loadButton_clicked();
        bool on_saveButton_clicked();
        void on_exportButton_clicked();
        //void on_quitButton_clicked();

        void slotActionSelectAll();

        void slotArrangeForceField(bool enabled);
        void slotArrangeRandom();
        void slotDecoTopBar(bool checked);
        void slotDecoBottomBar(bool checked);
        void slotDecoSetTitle();
        void slotDecoClearTitle();
        void slotHelpWebsite();
        void slotHelpWebsiteFetched();
        void slotHelpWebsiteFetchError();
        void slotHelpSupport();
        void slotHelpTutorial();
        void slotHelpUpdates();
        void slotSetBackMode(QAction* action);
        void slotSetBackRatio(QAction* action);

        void slotBackModeChanged();
        void slotBackRatioChanged();
        void slotShowPropertiesWidget(QWidget *);

        void slotVerifyTutorial(QNetworkReply * reply);
        void slotVerifySupport(/*const KnowledgeItemV1List & items*/);
        void slotVerifyVideoInputs(int count);
};

#endif
