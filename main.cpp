/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
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
#include <QTranslator>
#include <QLocale>
#include <QSettings>
#include <QtPlugin>
#include "FotoWall.h"
#include "RenderOpts.h"

#if !defined(QT_SHARED) && !defined(QT_DLL)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qsvg)
Q_IMPORT_PLUGIN(qtiff)
#endif

// init RenderOpts defaults
bool RenderOpts::LastMirrorEnabled = true;
bool RenderOpts::HQRendering = false;
bool RenderOpts::FirstRun = false;

int main( int argc, char ** args )
{
    // use the Raster GraphicsSystem as default on 4.5+
#if QT_VERSION >= 0x040500
    QApplication::setGraphicsSystem("raster");
#endif

    QApplication app(argc, args);
    app.setApplicationName("FotoWall");
    app.setApplicationVersion("0.5");
    app.setOrganizationName("Enrico Ros");

    QTranslator translator;
    translator.load( QString( ":/translations/fotowall_%1" ).arg( QLocale::system().name() ) );
    app.installTranslator(&translator);

    QSettings s;
    RenderOpts::FirstRun = s.value("fotowall/firstTime", true).toBool();
    s.setValue("fotowall/firstTime", false);

    FotoWall fw;
    fw.showMaximized();
    if(argc >0) {
        QString filePath = args[1];
        fw.load(filePath);
    }
    if (RenderOpts::FirstRun)
        fw.showIntroduction();

    return app.exec();
}
