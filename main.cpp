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
bool RenderOpts::HQRendering = false;
bool RenderOpts::FirstRun = false;
bool RenderOpts::OxygenStyleQuirks = false;
QColor RenderOpts::hiColor;

// proxy style, to change little stuff
#if QT_VERSION >= 0x040600
#include <QPainter>
#include <QProxyStyle>
#include <QStyleOption>
class FotowallStyle : public QProxyStyle {
public:
    void drawControl(ControlElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0) const
    {
        if (element == CE_RubberBand) {
            painter->save();
            QColor color = RenderOpts::hiColor;
            painter->setPen(color);
            color.setAlpha(80); painter->setBrush(color);
            painter->drawRect(option->rect.adjusted(0,0,-1,-1));
            painter->restore();
            return;
        }
        return QCommonStyle::drawControl(element, option, painter, widget);
    }
    int styleHint(StyleHint hint, const QStyleOption * option, const QWidget * widget, QStyleHintReturn * returnData) const
    {
        if (hint == SH_RubberBand_Mask)
            return false;
        return QCommonStyle::styleHint(hint, option, widget, returnData);
    }
};
#endif

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
    app.setApplicationVersion("0.7.0");
    app.setOrganizationName("Enrico Ros");
#if QT_VERSION >= 0x040600
    app.setStyle(new FotowallStyle);
#else
    RenderOpts::OxygenStyleQuirks = app.style()->objectName() == QLatin1String("oxygen");
#endif

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
    s.setValue("fotowall/firstTime", false);

    FotoWall fw;
    fw.showMaximized();
    QStringList images;
    for(int i=0; i<argc; i++) {
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
