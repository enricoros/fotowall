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

#ifndef __Settings_h__
#define __Settings_h__

#include <QList>
#include <QSettings>
#include <QStringList>
#include <QUrl>

class Settings : public QSettings
{
    public:
        Settings();
        ~Settings();

        // is this the first time Fotowall is executed
        bool firstTime() const;

        // the recent load/save history
        QList<QUrl> recentFotowallUrls() const;
        void addRecentFotowallUrl(const QUrl & url);
        void removeRecentFotowallUrl(const QUrl & url);

        // commandline files
        void addCommandlineUrl(const QString & url);
        QStringList commandlineUrls() const;

    private:
        bool m_firstTime;
        QList<QUrl> m_recentFotowallUrls;
        QStringList m_commandlineUrls;
};

#endif
