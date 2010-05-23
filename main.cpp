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
#include <QTime>
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

// lock orientation on symbian
#if defined(Q_OS_SYMBIAN)
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
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
#if !defined(Q_OS_MAC) && !defined(Q_OS_SYMBIAN)
    // use the Raster GraphicsSystem (but not on OsX and Symbian)
    QApplication::setGraphicsSystem("raster");
#endif

    // lock orientation on symbian
#if defined(Q_OS_SYMBIAN)
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
    TRAP_IGNORE(
    if (appUi)
        appUi->SetOrientationL(CAknAppUi::EAppUiOrientationLandscape);
    );
#endif
    QApplication app(argc, args);
    app.setApplicationName("Fotowall");
    app.setApplicationVersion("0.9.1");
    app.setOrganizationName("Enrico Ros");

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

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

    App::settings = new Settings(app.arguments().contains("-clearconfig"));
    RenderOpts::OxygenStyleQuirks = app.style()->objectName() == QLatin1String("oxygen");
    RenderOpts::hiColor = app.palette().color(QPalette::Highlight);
    VideoProvider::Disable = app.arguments().contains("-novideo");

    // startup video early.. disabled for production
    //VideoProvider::instance()->inputCount();

    for (int i = 1; i < argc; i++)
        if (App::isContentUrl(args[i]))
            App::settings->addCommandlineUrl(args[i]);

    MainWindow * mainWindow = new MainWindow;

    int mainLoopResult = app.exec();

    App::settings->sync();

    delete mainWindow;
    delete App::settings;
    return mainLoopResult;
}
