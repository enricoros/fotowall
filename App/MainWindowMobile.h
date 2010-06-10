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

#ifndef __MainWindowMobile_h__
#define __MainWindowMobile_h__

#include "Shared/PlugGui/Container.h"
class QNetworkAccessManager;
class PictureSearchWidget;
class SceneView;
class SmartPanel;
class TopbarContainer;
class Workflow;

class MainWindowMobile : public PlugGui::Container
{
    Q_OBJECT
    public:
        MainWindowMobile(QWidget * parent = 0);
        ~MainWindowMobile();

    protected:
        // ::PlugGui::Container
        QSize sceneViewSize() const;
        void applianceSetTitle(const QString & title);
        void applianceSetScene(AbstractScene * scene);
        void applianceSetTopbar(const QList<QWidget *> & widgets);
        void applianceSetSidebar(QWidget * widget);
        void applianceSetCentralwidget(QWidget * widget);
        void applianceSetValue(quint32 key, const QVariant & value);
        void applianceSetFocusToScene();

        // ::QWidget
        void closeEvent(QCloseEvent *);
        void resizeEvent(QResizeEvent *);

    private:
        QNetworkAccessManager * m_networkAccessManager;
        PictureSearchWidget *   m_pictureSearch;
        SceneView *             m_sceneView;
        TopbarContainer *       m_topbarContainer;
        QList<SmartPanel *>     m_smartPanels;

    private Q_SLOTS:
        void slotClosePictureSearch();
};

#endif
