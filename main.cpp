/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2008 by Enrico Ros <enrico.ros@gmail.com>          *
 * Modified by Tanguy Arnaud <phparnsk8@gmail.com>, see CHANGLOG to have   *
 * summary of the modification.										       * 
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QString>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include "FotoWall.h"

int main( int argc, char ** args )
{
	QApplication app(argc, args);
	QString locale = QLocale::system().name().section('_', 0, 0);

	QTranslator translator;
	translator.load(QString("fotowall_") + locale);
	app.installTranslator(&translator);

	QTranslator qtTranslator;
	qtTranslator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	app.installTranslator(&qtTranslator);


	FotoWall fw;
	fw.showMaximized();
	return app.exec();
}
