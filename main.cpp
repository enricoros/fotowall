/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QFont>
#include <QLocale>
#include <QLibraryInfo>
#include <QSettings>
#include <QStyle>
#include <QTime>
#include <QtPlugin>
#include "App/App.h"
#include "App/Settings.h"
#include "Shared/RenderOpts.h"
#include "Shared/VideoProvider.h"

#if defined(MOBILE_UI)
#include "App/MainWindowMobile.h"
#else
#include "App/MainWindow.h"
#endif

#if defined(HAS_TRANSLATIONS)
#include <QTranslator>
#endif

// init RenderOpts defaults
bool RenderOpts::LastMirrored = true;
bool RenderOpts::HQRendering = false;
bool RenderOpts::ARGBWindow = false;
bool RenderOpts::PDFExporting = false;
bool RenderOpts::OpenGLWindow = false;
bool RenderOpts::OxygenStyleQuirks = false;
bool VideoProvider::Disable = false;
QColor RenderOpts::hiColor;

int main( int argc, char ** args )
{
    // use the Raster GraphicsSystem on X11
#if defined(Q_WS_X11)
    QApplication::setGraphicsSystem("raster");
#endif

    QApplication app(argc, args);
    app.setApplicationName("Fotowall");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Enrico Ros");

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

#if defined(HAS_TRANSLATIONS)
    // translate fotowall + default-qt messages
    QString locale;
    QStringList arguments = app.arguments();
    // Check for specified locale, or use the system one.
    int pos=arguments.indexOf("-locale");
    if (pos != -1 && pos < arguments.size()-1) locale = arguments.at(pos+1);
    else locale = QLocale::system().name();

    QTranslator translator;
    translator.load(QString(":/translations/fotowall_%1").arg(locale));
    app.installTranslator(&translator);
    QTranslator qtTranslator;
    qtTranslator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);
#endif

    App::settings = new Settings(app.arguments().contains("-clearconfig"));
    RenderOpts::OxygenStyleQuirks = app.style()->objectName() == QLatin1String("oxygen");
    RenderOpts::hiColor = app.palette().color(QPalette::Highlight);
    VideoProvider::Disable = app.arguments().contains("-novideo");

    // startup video early.. disabled for production
    //VideoProvider::instance()->inputCount();

    for (int i = 1; i < argc; i++)
        if (App::isContentUrl(args[i]))
            App::settings->addCommandlineUrl(args[i]);

#if defined(MOBILE_UI)
    QFont smallFont = app.font();
    smallFont.setPointSize(6);
    app.setFont(smallFont);

    MainWindowMobile * mainWindow = new MainWindowMobile;
    mainWindow->setFont(smallFont);
#else
    MainWindow * mainWindow = new MainWindow;
#endif

    int mainLoopResult = app.exec();

    App::settings->sync();

    delete mainWindow;
    delete App::settings;
    return mainLoopResult;
}
