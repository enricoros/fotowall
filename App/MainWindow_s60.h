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

#ifndef __MainWindow_s60_h__
#define __MainWindow_s60_h__

#include "Shared/PlugGui/Container.h"
namespace Ui { class MainWindow; }
class QGridLayout;
class QNetworkAccessManager;
class PictureSearchWidget;
class Workflow;

class MainWindowMobile : public PlugGui::Container
{
    Q_OBJECT
    public:
        MainWindowMobile(QWidget * parent = 0);
        ~MainWindowMobile();

    protected:
        // ::Appliance::Container
        QSize sceneViewSize() const;
        void applianceSetTitle(const QString & title);
        void applianceSetScene(AbstractScene * scene);
        void applianceSetTopbar(const QList<QWidget *> & widgets);
        void applianceSetSidebar(QWidget * widget);
        void applianceSetCentralwidget(QWidget * widget);
        void applianceSetValue(quint32 key, const QVariant & value);
        void applianceSetFocusToScene();

        // ::QWidget
        void closeEvent(QCloseEvent * event);

    private:
        void addNavigationWidget(QWidget * widget, int row, Qt::Alignment alignment);
        void removeNavigationWidget(QWidget * widget);

        Ui::MainWindow *        ui;
        QNetworkAccessManager * m_networkAccessManager;
        QGridLayout *           m_navigationLayout;
        PictureSearchWidget *   m_pictureSearch;

    private Q_SLOTS:
        // notifications
        void slotClosePictureSearch();
        void slotHelpBarClicked(quint32);
};

#endif
