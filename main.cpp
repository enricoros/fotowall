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
#include <QLocale>
#include <QLibraryInfo>
#include <QSettings>
#include <QStyle>
#include <QTranslator>
#include <QtPlugin>
#include "App/App.h"
#include "App/MainWindow.h"
#include "App/Settings.h"
#include "Shared/RenderOpts.h"
#include "Shared/VideoProvider.h"

#if defined(STATIC_LINK)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qsvg)
Q_IMPORT_PLUGIN(qtiff)
#endif

// init RenderOpts defaults
bool RenderOpts::LastMirrored = true;
bool RenderOpts::HQRendering = false;
bool RenderOpts::ARGBWindow = false;
bool RenderOpts::OpenGLWindow = false;
bool RenderOpts::OxygenStyleQuirks = false;
bool VideoProvider::Disable = false;
QColor RenderOpts::hiColor;

int main( int argc, char ** args )
{
#if !defined(Q_OS_MAC) // raster on OSX == b0rken
    // use the Raster GraphicsSystem as default on 4.5+
#if QT_VERSION >= 0x040500
    QApplication::setGraphicsSystem("raster");
#endif
#endif

    QApplication app(argc, args);
    app.setApplicationName("Fotowall");
    app.setApplicationVersion("0.8.96");
    app.setOrganizationName("Enrico Ros");
    RenderOpts::OxygenStyleQuirks = app.style()->objectName() == QLatin1String("oxygen");

    // translate fotowall + default-qt messages
    QString locale =  QLocale::system().name();
    QTranslator translator;
    translator.load(QString(":/translations/fotowall_%1").arg(locale));
    app.installTranslator(&translator);
    QTranslator qtTranslator;
    qtTranslator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    App::settings = new Settings;
    RenderOpts::hiColor = app.palette().color(QPalette::Highlight);
    VideoProvider::Disable = app.arguments().contains("-novideo");
    App::settings->setValue("Fotowall/FirstTime", false);

    QStringList urls;
    for (int i = 1; i < argc; i++)
        if (App::isContentUrl(args[i]))
            urls.append(args[i]);

    App::mainWindow = new MainWindow(urls);

    int mainLoopResult = app.exec();

    App::settings->sync();

    delete App::mainWindow;
    delete App::settings;
    return mainLoopResult;
}
