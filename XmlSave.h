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

#ifndef __Save__
#define __Save__
 
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>
#include <QList>

class PictureContent;
class TextContent;
class AbstractContent;

class XmlSave : public QObject
{
    Q_OBJECT
    public:
        XmlSave(const QString &);
        ~XmlSave();
        void saveProject(QString, int);
        void saveImage(PictureContent *);
        void saveText(TextContent *);

    private :
        QDomDocument doc;
        QDomElement m_rootElement;
        QDomElement m_imageElements;
        QDomElement m_textElements;
        QDomElement m_projectElement;
        QFile file;
        QTextStream out;

        void saveAbstractContent(AbstractContent *content, QDomElement &parentElement);
};

#endif 

