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
#include <QFile>
#include <QTextStream>
#include <QList>
#include "PictureContent.h"
#include "TextContent.h"
#include "ModeInfo.h"

class Desk;
class AbstractContent;
class PictureContent;
class TextContent;
class FotoWall;

class XmlRead : public QObject
{
    Q_OBJECT
    public:
        XmlRead(const QString &, Desk *);
        ~XmlRead();
        void readProject(FotoWall *);
        void readDesk();
        void readImages();
        void readText();

    private:
        void readAbstractContent(AbstractContent *, QDomElement &);
        // Clear the desk, make ready for restoration
        void prepareRestore();
    private :
        Desk *m_desk;
        QDomDocument doc;
        QDomElement m_projectElement;
        QDomElement m_deskElement;
        QDomElement m_imagesElement;
        QDomElement m_textsElement;
        QFile file;
        QTextStream out;

        QString m_projectTitle;
        int m_projectMode;

    Q_SIGNALS:
        void changeMode(int);
        void setModeInfo(ModeInfo);

};

#endif 

