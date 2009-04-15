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

#include "XmlRead.h"
#include <QString>
#include <QDebug>
#include <QStringList>


XmlRead::XmlRead(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Can't open the file " << filePath;
        return;
    }

    QString *error = new QString();
    if (!doc.setContent(&file, false, error)) {        //établit le document XML à
        qDebug() << "Can't set content : " << *error;
        file.close();                   //partir des données du fichier (hiérarchie, etc)
        return;
    }
    file.close();

    QDomElement root = doc.documentElement(); // The root node 
    m_projectElement = root.firstChildElement("project"); // Get the project node
    // Get the parent images node (containing all the images)
    m_imagesElement = root.firstChildElement("images"); 
}

XmlRead::~XmlRead() 
{
}

void XmlRead::readProject()
{
    if (!m_projectElement.isNull()) { 
        m_projectTitle = m_projectElement.firstChildElement("title").text();
        m_projectMode = m_projectElement.firstChildElement("mode").text().toInt();
    }
}

QStringList XmlRead::readImage()
{
    QStringList images;

    QDomNode node = m_imagesElement.firstChild();
    QDomElement imageElement; // An image element (just one image)
    // Foreach image nodes
    while (!node.isNull()) {
        imageElement = node.toElement();
        QStringList image;
        image << imageElement.firstChildElement("name").text() 
            <<  imageElement.firstChildElement("path").text()
            << imageElement.firstChildElement("effects").text();
        images << image;
        node = node.nextSibling();
    }
    return images;
}

QString XmlRead::getProjectTitle() const
{
    return m_projectTitle;
}
int XmlRead::getProjectMode() const
{
    return m_projectMode;
}

