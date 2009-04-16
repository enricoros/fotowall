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
#include <QGraphicsView>
#include "Desk.h"
#include "frames/FrameFactory.h"


XmlRead::XmlRead(const QString &filePath, Desk *desk) : m_desk(desk)
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
    m_textsElement = root.firstChildElement("texts"); 

    prepareRestore();
}

XmlRead::~XmlRead() 
{
}

void XmlRead::readProject()
{
//    FIXME : Save and load these elements (in Desk ) !
//    QColor color;
//    data >> color;
//    m_titleColorPicker->setColor(color);
//    data >> color;
//    m_foreColorPicker->setColor(color);
//    data >> color;
//    m_grad1ColorPicker->setColor(color);
//    data >> color;
//    m_grad2ColorPicker->setColor(color);
//    // FIXME: restore background
    if (!m_projectElement.isNull()) { 
        m_desk->setTitleText(m_projectElement.firstChildElement("title").text());
        int mode = m_projectElement.firstChildElement("mode").text().toInt() - 1;
        emit changeMode(mode); 
    }
}

void XmlRead::prepareRestore()
{
    qDeleteAll(m_desk->m_content);
    m_desk->m_content.clear();
    m_desk->m_backContent = 0;
}

void XmlRead::readAbstractContent(AbstractContent *content, QDomElement &parentElement)
{
    content->prepareGeometryChange();

    QDomElement domElement;
    // Load image size saved in the rect node
    domElement = parentElement.firstChildElement("rect");
    int x, y, w, h;
    x = domElement.firstChildElement("x").text().toInt();
    y = domElement.firstChildElement("y").text().toInt();
    w = domElement.firstChildElement("w").text().toInt();
    h = domElement.firstChildElement("h").text().toInt();
    QRect rect(x, y, w, h);
    content->m_rect = rect;
    content->layoutChildren();

    // Load position coordinates
    domElement = parentElement.firstChildElement("pos");
    x = domElement.firstChildElement("x").text().toInt();
    y = domElement.firstChildElement("y").text().toInt();
    content->setPos(x, y);
    
    /* FIXME : load transform. Before it was done like this :
        QTransform t;
        data >> t;
        setTransform(t);
        */

    int zvalue = parentElement.firstChildElement("zvalue").text().toInt();
    content->setZValue(zvalue);

    bool visible = parentElement.firstChildElement("visible").text().toInt(); 
    content->setVisible(visible);

    bool hasText = parentElement.firstChildElement("frame-text-enabled").text().toInt(); 
    content->setFrameTextEnabled(hasText);
    if(hasText) {
        QString text = parentElement.firstChildElement("frame-text").text();
        content->setFrameText(text);
    }

    quint32 frameClass = parentElement.firstChildElement("frame-class").text().toInt();
    content->setFrame(frameClass ? FrameFactory::createFrame(frameClass) : 0);
    content->update();

    // Restore transformations
    domElement = parentElement.firstChildElement("transformation");
    int z;
    x = domElement.firstChildElement("x-rotation").text().toInt();
    y = domElement.firstChildElement("y-rotation").text().toInt();
    z = domElement.firstChildElement("z-rotation").text().toInt();
    content->setRotation(x, y, z);
}

void XmlRead::readImages()
{
    QDomNode node = m_imagesElement.firstChild();
    QDomElement imageElement; // An image element (just one image)
    QString name, path;
    // Foreach image nodes
    while (!node.isNull()) {
        imageElement = node.toElement();
        
        // Create image item (connect slots...).
        PictureContent *content = m_desk->createPicture(QPoint());

        // Reload general properties (shared by all items)
        readAbstractContent(content, imageElement);

        name = imageElement.firstChildElement("name").text();
        path = imageElement.firstChildElement("path").text();
        content->loadPhoto(path);

        QStringList effects =  imageElement.firstChildElement("effects").text().split(" ");
        foreach(QString effect, effects) {
            if(!effect.isEmpty())
                content->setEffect(effect.toInt());
        }

        //Read next node
        node = node.nextSibling();
    }
}

void XmlRead::readText()
{
    QDomNode node = m_textsElement.firstChild();
    QDomElement textElement; // An image element (just one image)
    QString text;
    // Foreach image nodes
    while (!node.isNull()) {
        textElement = node.toElement();

        // Create image item (connect slots...).
        TextContent *content = m_desk->createText(QPoint());

        // Reload general properties (shared by all items)
        readAbstractContent(content, textElement);

        text = textElement.firstChildElement("html-text").text();
        content->setHtml(text);

        //Read next node
        node = node.nextSibling();
    }

}
