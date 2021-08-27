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

#ifndef __CanvasAppliance_h__
#define __CanvasAppliance_h__

#include <QActionGroup>

#include "Shared/PlugGui/AbstractAppliance.h"
#include "ui_CanvasAppliance.h"
class QMenu;
class AbstractContent;
class Canvas;
class PixmapButton;

class CanvasAppliance : public QObject, public PlugGui::AbstractAppliance
{
    Q_OBJECT
    public:
        CanvasAppliance(Canvas * extCanvas, QObject * parent = 0);
        ~CanvasAppliance();

        // take the canvas (NOTE: IMMEDIATELY DELETE AFTER THIS)
        Canvas * takeCanvas();

        // ::Appliance::AbstractAppliance
        QString applianceName() const { return tr("Canvas"); }
        bool applianceCommand(int command);
        bool appliancePendingChanges() const;
        bool applianceSave(const QString & filePath = QString());
        void setProjectMode(const int mode);

    private:
        QMenu * createArrangeMenu();
        QMenu * createBackgroundMenu();
        QMenu * createDecorationMenu();
        void setNormalProject();
        void setExactSizeProject(bool usePrevious);
        void setWallpaperProject();
        void setCDProject();
        void setDVDProject();
        void configurePrint(bool enabled);

    private:
        Ui::CanvasApplianceElements ui;
        Canvas *                    m_extCanvas;
        QWidget *                   m_dummyWidget;
        QActionGroup *              m_gBackModeGroup;
        QActionGroup *              m_gBackRatioGroup;
        QAction *                   m_gBackContentAction;
        QList<PixmapButton *>       m_webcamButtons;

    private Q_SLOTS:

        // actions in the Add contents box
        void slotAddCanvas();
        void slotAddPicture();
        void slotAddText();
        void slotAddWebcam();
        void slotAddWordcloud();
        void slotSearchPicturesToggled(bool on);

        // actions in the Canvas box
        void slotBackContentRemove(bool checked);
        void slotProjectComboActivated(int index);
        void slotSetBackMode(QAction* action);
        void slotSetBackRatio(QAction* action);
        void slotArrangeForceField(bool enabled);
        void slotArrangeColorCollage();
        void slotArrangeRandom();
        void slotArrangeShaped();
        void slotDecoTopBar(bool checked);
        void slotDecoBottomBar(bool checked);
        void slotDecoSetTitle();
        void slotDecoClearTitle();

        // actions in the File box
        bool slotFileLoad();
        bool slotFileSave();
        bool slotFileExport();

        // signals from the canvas
        void slotEditContent(AbstractContent * content);
        void slotBackConfigChanged();
        void slotShowPropertiesWidget(QWidget *);
        void slotFilePathChanged();

        // other actions
        void slotVerifyVideoInputs(int count);
};

#endif
