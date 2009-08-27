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
#include <QLibraryInfo>
#include <QSettings>
#include <QStyle>
#include <QtPlugin>
#include "FotoWall.h"
#include "RenderOpts.h"

#if defined(STATIC_LINK)
Q_IMPORT_PLUGIN(qgif)
Q_IMPORT_PLUGIN(qjpeg)
Q_IMPORT_PLUGIN(qsvg)
Q_IMPORT_PLUGIN(qtiff)
#endif

// init RenderOpts defaults
bool RenderOpts::LastMirrorEnabled = true;
bool RenderOpts::ARGBWindow = false;
bool RenderOpts::HQRendering = false;
bool RenderOpts::FirstRun = false;
bool RenderOpts::OxygenStyleQuirks = false;
bool RenderOpts::DisableVideoProvider = false;
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
    app.setApplicationName("FotoWall");
    app.setApplicationVersion("0.7.4");
    app.setOrganizationName("Enrico Ros");
    RenderOpts::OxygenStyleQuirks = app.style()->objectName() == QLatin1String("oxygen");

    // translate fotowall + default-qt messages
    QString locale =  QLocale::system().name();
    QTranslator translator;
    translator.load(QString( ":/translations/fotowall_%1" ).arg(locale));
    app.installTranslator(&translator);
    QTranslator qtTranslator;
    qtTranslator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QSettings s;
    RenderOpts::FirstRun = s.value("fotowall/firstTime", true).toBool();
    RenderOpts::hiColor = app.palette().color(QPalette::Highlight);
    RenderOpts::DisableVideoProvider = app.arguments().contains("-novideo");
    s.setValue("fotowall/firstTime", false);

    FotoWall fw;
    fw.showMaximized();
    QStringList images;
    for (int i = 1; i < argc; i++) {
        QString filePath = args[i];
        if (filePath.endsWith(".fotowall", Qt::CaseInsensitive))
            fw.loadXml(filePath);
        else
            images << filePath;
    }
    fw.loadImages(images);

    if (RenderOpts::FirstRun)
        fw.showIntroduction();

    return app.exec();
}
