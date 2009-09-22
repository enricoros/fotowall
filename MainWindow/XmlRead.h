/*****************************************************************************
* Copyright (C) 2008 TANGUY Arnaud <arn.tanguy@gmail.com>                    *
*                                                                            *
* This program is free software; you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation; either version 2 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License along    *
* with this program; if not, write to the Free Software Foundation, Inc.,    *
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                *
******************************************************************************/

#ifndef __XmlRead__
#define __XmlRead__

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include "MainWindow/ModeInfo.h"

class Desk;
class AbstractContent;
class MainWindow;
class PictureContent;
class TextContent;

class XmlRead : public QObject
{
    Q_OBJECT
    public:
        XmlRead(const QString & filePath);
        void readProject(MainWindow * mainWindow);
        void readDesk(Desk * desk);
        void readContent(Desk * desk);

    private :
        QDomElement m_projectElement;
        QDomElement m_deskElement;
        QDomElement m_contentElement;

    Q_SIGNALS:
        void changeMode(int);
        void setModeInfo(ModeInfo);
};

#endif

