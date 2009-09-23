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

#ifndef __XmlSave_h__
#define __XmlSave_h__

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "App/ModeInfo.h"

class Desk;

class XmlSave : public QObject
{
    public:
        XmlSave();

        bool writeFile(const QString & filePath);

        void saveContent(const Desk *);
        void saveDesk(const Desk *);
        void saveProject(int, const ModeInfo&);

    private:
        QDomDocument doc;
        QDomElement m_rootElement;
        QDomElement m_contentElements;
        QDomElement m_projectElement;
        QDomElement m_deskElement;
};

#endif
