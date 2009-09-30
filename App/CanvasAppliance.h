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

#include "Shared/AbstractAppliance.h"
#include "Canvas/Canvas.h"
#include "ui_CanvasAppliance.h"
class QMenu;
class SceneView;

class CanvasAppliance : public Appliance::AbstractAppliance
{
    Q_OBJECT
    public:
        CanvasAppliance(Canvas * canvas, SceneView * view, QObject * parent = 0);

        // peek the internal canvas
        Canvas * canvas() const;

        // ::Appliance::AbstractAppliance
        QString applianceName() const { return tr("Canvas"); }
        int applianceElements() const { return Appliance::UseAllElements; }

    private:
        QMenu * createArrangeMenu();
        QMenu * createBackgroundMenu();
        QMenu * createDecorationMenu();
        void setNormalProject();
        void setCDProject();
        void setDVDProject();
        void setExactSizeProject();

    private:
        Canvas * m_canvas;

        Ui::CanvasApplianceElements * ui;
        QWidget * m_dummyWidget;

        QActionGroup *  m_gBackActions;
        QActionGroup *  m_gBackRatioActions;

    private Q_SLOTS:

        // actions in the add contents box
        void slotAddCanvas();
        void slotAddFlickrToggled(bool on);
        void slotAddPicture();
        void slotAddText();
        void slotAddWebcam();
        void slotAddWordCloud();

        // actions in the canvas box
        void slotProjectTypeActivated(int index);
        void slotSetBackMode(QAction* action);
        void slotSetBackRatio(QAction* action);
        void slotArrangeForceField(bool enabled);
        void slotArrangeRandom();
        void slotDecoTopBar(bool checked);
        void slotDecoBottomBar(bool checked);
        void slotDecoSetTitle();
        void slotDecoClearTitle();

        // notifications from the canvas
        void slotRefreshCanvas();
        void slotBackModeChanged();
        void slotBackRatioChanged();

        void slotShowPropertiesWidget(QWidget *);

        void slotVerifyVideoInputs(int count);
};

#endif
