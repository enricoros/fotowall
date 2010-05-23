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

#include "App/MainWindow_s60.h"

#include "Shared/BreadCrumbBar.h"
#include "Shared/ButtonsDialog.h"
#include "Shared/RenderOpts.h"
#include "App.h"
#include "OnlineServices.h"
#include "PictureSearchWidget.h"
#include "SceneView.h"
#include "Settings.h"
#include "Workflow.h"
#include "ui_MainWindow_s60.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFont>
#include <QNetworkAccessManager>
#include <QVariant>

MainWindow::MainWindow(QWidget * parent)
    : PlugGui::Container(parent)
    , ui(new Ui::MainWindow())
    , m_networkAccessManager(new QNetworkAccessManager)
    , m_navigationLayout(0)
    , m_pictureSearch(0)
{
    // setup widget
    applianceSetTitle(QString());
#if !defined(Q_OS_SYMBIAN)
    setWindowIcon(QIcon(":/data/fotowall.png"));
#endif

    // smaller font on symbians
#if defined(Q_OS_SYMBIAN)
    QFont smallerFont;
    smallerFont.setPointSize(8);
    QApplication::setFont(smallerFont);
#endif

    // init ui
    ui->setupUi(this);
    ui->topBar->setFixedHeight(App::TopBarHeight);
    ui->applianceSidebar->hide();
    ui->sceneView->setFocus();

    // create the navigation layout
    m_navigationLayout = new QGridLayout;
    m_navigationLayout->setContentsMargins(0, 0, 0, 0);
    m_navigationLayout->setSpacing(3);
    ui->navBar->setLayout(m_navigationLayout);

    // create the Workflow navigation bar
    BreadCrumbBar * workflowBar = new BreadCrumbBar(ui->sceneView);
    workflowBar->setObjectName(QString::fromUtf8("applianceNavBar"));
    workflowBar->setClickableLeaves(false);
    workflowBar->setBackgroundOffset(-1);
    addNavigationWidget(workflowBar, 0, Qt::AlignLeft);

    // create the Help bar
    BreadCrumbBar * helpBar = new BreadCrumbBar(ui->sceneView);
    connect(helpBar, SIGNAL(nodeClicked(quint32)), this, SLOT(slotHelpBarClicked(quint32)));
    helpBar->setBackgroundOffset(1);
    helpBar->addNode(1, tr(" ? "), 0);
    addNavigationWidget(helpBar, 0, Qt::AlignRight);

    // show (with last geometry)
#if defined(Q_OS_SYMBIAN)
    showFullScreen();
#else
    if (!restoreGeometry(App::settings->value("Fotowall/Geometry").toByteArray())) {
        QRect desktopGeometry = QApplication::desktop()->availableGeometry();
        resize(2 * desktopGeometry.width() / 3, 2 * desktopGeometry.height() / 3);
        showMaximized();
    } else
        show();
#endif


    // start the workflow
    new Workflow((PlugGui::Container *)this, workflowBar);

    // create the online services
    new OnlineServices(m_networkAccessManager);

#if 0
    // start with the Help appliance the first time
    if (App::settings->firstTime())
        App::workflow->stackHelpAppliance();
#endif
}

MainWindow::~MainWindow()
{
    // save window geometry
    if (!isMaximized() && !isFullScreen())
        App::settings->setValue("Fotowall/Geometry", saveGeometry());
    else
        App::settings->remove("Fotowall/Geometry");

    // delete everything
    delete App::workflow;
    delete App::onlineServices;
    delete m_networkAccessManager;
    delete ui;
}

QSize MainWindow::sceneViewSize() const
{
    return ui->sceneView->viewport()->size();
}

void MainWindow::applianceSetTitle(const QString & title)
{
    QString tString = title.isEmpty() ? QString() : title + " - ";
    tString += QCoreApplication::applicationName() + " ";
    if (title.isEmpty())
        tString += "' Alchimia ' ";
    tString += QCoreApplication::applicationVersion();
#if QT_VERSION < 0x040500
    tString += "   -Limited Edition (Qt 4.4)-";
#endif
    setWindowTitle(tString);
}

void MainWindow::applianceSetScene(AbstractScene * scene)
{
    ui->sceneView->setScene(scene);
}

static void removeLayoutChildWidges(QLayout * layout)
{
    while (QLayoutItem * item = layout->takeAt(0)) {
        if (QWidget * oldWidget = item->widget())
            oldWidget->setVisible(false);
        delete item;
    }
}

void MainWindow::applianceSetTopbar(const QList<QWidget *> & widgets)
{
    // clear the topbar layout hiding all widgets
    removeLayoutChildWidges(ui->applianceLeftBarLayout);
    removeLayoutChildWidges(ui->applianceRightBarLayout);

    // add the widgets to the topbar and show them
    foreach (QWidget * widget, widgets) {
        if (widget->property("@rightBar").toBool())
            ui->applianceRightBarLayout->addWidget(widget);
        else
            ui->applianceLeftBarLayout->addWidget(widget);
        widget->setFixedHeight(App::TopBarHeight);
        widget->setVisible(true);
    }
}

void MainWindow::applianceSetSidebar(QWidget * widget)
{
    // clear the sidebar layout hiding any widget
    removeLayoutChildWidges(ui->applianceSidebarLayout);

    // completely hide the sidebar if no widget
    ui->applianceSidebar->setVisible(widget);

    // if any, add the widget to the sidebar
    if (widget) {
        ui->applianceSidebarLayout->addWidget(widget);
        widget->show();
    }
}

void MainWindow::applianceSetCentralwidget(QWidget * widget)
{
    if (widget)
        qWarning("MainWindow::applianceSetCentralwidget: unsupported");
}

void MainWindow::applianceSetValue(quint32 key, const QVariant & value)
{
    if (key == App::CC_ShowPictureSearch) {

        // destroy if needed
        bool visible = value.toBool();
        if (!visible && m_pictureSearch) {
            m_pictureSearch->deleteLater();
            m_pictureSearch = 0;
            return;
        }

        // create if needed
        if (visible && !m_pictureSearch) {
            m_pictureSearch = new PictureSearchWidget(m_networkAccessManager);
            connect(m_pictureSearch, SIGNAL(requestClosure()), this, SLOT(slotClosePictureSearch()), Qt::QueuedConnection);
            m_pictureSearch->setWindowIcon(QIcon(":/data/insert-download.png"));
            m_pictureSearch->setWindowTitle(tr("Search Web Pictures"));
            m_pictureSearch->setWindowFlags(Qt::Tool);
            QPoint newPos = mapToGlobal(ui->sceneView->pos()) + QPoint(-20, 10);
            if (newPos.x() < 0)
                newPos.setX(0);
            m_pictureSearch->move(newPos);
            m_pictureSearch->layout()->activate();
            m_pictureSearch->resize(50, 50);
            m_pictureSearch->show();
            m_pictureSearch->setFocus();
            return;
        }

    } else
        qWarning("MainWindow::applianceSetValue: unknown key 0x%x", key);
}

void MainWindow::applianceSetFocusToScene()
{
    ui->sceneView->setFocus(Qt::OtherFocusReason);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    event->setAccepted(App::workflow->requestExit());
}

void MainWindow::slotClosePictureSearch()
{
    App::workflow->applianceCommand(App::AC_ClosePicureSearch);
}

void MainWindow::slotHelpBarClicked(quint32 id)
{
    if (id == 1)
        App::workflow->stackHelpAppliance();
}

void MainWindow::addNavigationWidget(QWidget * widget, int row, Qt::Alignment alignment)
{
    // add the widget at the right place in the grid layout
    if (alignment == Qt::AlignRight)
        m_navigationLayout->addWidget(widget, row, 1, 1, 1, Qt::AlignRight);
    else if (alignment & Qt::AlignHCenter)
        m_navigationLayout->addWidget(widget, row, 0, 1, 2, Qt::AlignLeft);
    else
        m_navigationLayout->addWidget(widget, row, 0, 1, 1, Qt::AlignLeft);

    // ensure the widget is shown
    widget->show();
}

void MainWindow::removeNavigationWidget(QWidget *widget)
{
    widget->hide();
    m_navigationLayout->removeWidget(widget);
    widget->setParent(0);
}
