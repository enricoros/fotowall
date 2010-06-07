/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __App_h__
#define __App_h__

#include <QObject>
#include <QUrl>
class AbstractPictureService;
class OnlineServices;
class Settings;
class Workflow;

#define HERE qWarning("> %s %d: %s", __FILE__, __LINE__, __FUNCTION__);

class App
{
    public:
        // uniquely instanced objects
        static Settings * settings;
        static Workflow * workflow;
        static OnlineServices * onlineServices;
        static AbstractPictureService * pictureService;

        // consts
        static const int TopBarHeight = 80;

        // commands understood by container
        enum {
            CC_ShowPictureSearch    = 0x0001,
            CC_HideTopBar           = 0x0002,
        };

        // commands understood by appliances
        enum {
            AC_ClearBackground      = 0x0001,
            AC_ClosePicureSearch    = 0x0002
        };

        // utility functions
        static QString supportedImageFormats();
        static bool isPictureFile(const QString & picFilePath);
        static bool isFotowallFile(const QString & fwFilePath);
        static bool isContentUrl(const QString & url);
        static bool validateFotowallUrl(const QString & url);
};

#endif
