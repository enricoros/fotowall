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
#include "FotoWall.h"

#define notImplemented() {qWarning("%s:%d: %s NOT Implemented!", __FILE__, __LINE__, __FUNCTION__);}

int main( int argc, char ** args )
{
    QApplication app(argc, args);

    QTranslator translator;
    translator.load( QString( "translations/fotowall_%1" ).arg( QLocale::system().name() ) );
    app.installTranslator(&translator);

    FotoWall fw;
    fw.show();
    return app.exec();
}
