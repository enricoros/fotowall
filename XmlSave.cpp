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

#include "XmlSave.h"
#include <QFile>
#include <QDebug>
#include "PictureContent.h"
#include "TextContent.h"
#include "CPixmap.h"

XmlSave::XmlSave(const QString &filePath)
{
    // This element contains all the others.
    m_rootElement = doc.createElement("fotowall");
    // This is general informations about the project (title...)
    m_projectElement = doc.createElement("project");
    // All the images will be saved in this element
    m_imageElements = doc.createElement("images");
    // All the texts here
    m_textElements = doc.createElement("texts");
    // Add elements to the root node (fotowall).
    m_rootElement.appendChild(m_projectElement);
    m_rootElement.appendChild(m_imageElements);
    m_rootElement.appendChild(m_textElements);

    // Add the root (and all the sub-nodes) to the document
    doc.appendChild(m_rootElement);

    // Open layout file
    file.setFileName(filePath);
    if (!file.open(QIODevice::WriteOnly))
        return;
    out.setDevice(&file);
}

XmlSave::~XmlSave()
{
   //Add at the begining : <?xml version="1.0" ?>
   QDomNode noeud = doc.createProcessingInstruction("xml","version=\"1.0\" ");
   doc.insertBefore(noeud,doc.firstChild());
   //save in the file (4 spaces indent) 
   doc.save(out,4);
   file.close();
}

void XmlSave::saveProject(QString title, int mode)
{
    // Title element
    QDomElement titleElement = doc.createElement("title");
    m_projectElement.appendChild(titleElement);
    QDomText titleText = doc.createTextNode(title);   
    titleElement.appendChild(titleText);             

    // Mode element
    QDomElement modeElement = doc.createElement("mode");
    m_projectElement.appendChild(modeElement);
    QString modeStr; modeStr.setNum(mode);
    QDomText modeText = doc.createTextNode(modeStr);   
    modeElement.appendChild(modeText);             
}

void XmlSave::saveAbstractContent(AbstractContent *content, QDomElement &parentElement)
{
    // Save general item properties
    
    QDomElement domElement; 
    QDomText text;
    QString valueStr; 

    // Save item position and size
    QDomElement rectParent = doc.createElement("rect");
    QDomElement xElement= doc.createElement("x");
    rectParent.appendChild(xElement);
    QDomElement yElement= doc.createElement("y");
    rectParent.appendChild(yElement);
    QDomElement wElement= doc.createElement("w");
    rectParent.appendChild(wElement);
    QDomElement hElement= doc.createElement("h");
    rectParent.appendChild(hElement);

    QRectF rect = content->boundingRect();
    QString x, y, w, h;
    x.setNum(rect.x()); y.setNum(rect.y());
    w.setNum(rect.width()); h.setNum(rect.height());
    xElement.appendChild(doc.createTextNode(x));             
    yElement.appendChild(doc.createTextNode(y));             
    wElement.appendChild(doc.createTextNode(w));             
    hElement.appendChild(doc.createTextNode(h));             
    parentElement.appendChild(rectParent);

    // Save the position
    domElement= doc.createElement("pos");
    xElement = doc.createElement("x");
    yElement = doc.createElement("y");
    valueStr.setNum(content->pos().x());
    xElement.appendChild(doc.createTextNode(valueStr));             
    valueStr.setNum(content->pos().y());
    yElement.appendChild(doc.createTextNode(valueStr));             
    domElement.appendChild(xElement);
    domElement.appendChild(yElement);
    parentElement.appendChild(domElement);

    // Save the stacking position
    domElement= doc.createElement("zvalue");
    parentElement.appendChild(domElement);
    valueStr.setNum(content->zValue());
    text = doc.createTextNode(valueStr);   
    domElement.appendChild(text);             

    // Save the visible state 
    domElement= doc.createElement("visible");
    parentElement.appendChild(domElement);
    valueStr.setNum(content->isVisible());
    text = doc.createTextNode(valueStr);   
    domElement.appendChild(text);             

    // Save the frame class
    valueStr.setNum(content->frameClass());
    domElement= doc.createElement("frame-class");
    parentElement.appendChild(domElement);
    text = doc.createTextNode(valueStr);   
    domElement.appendChild(text);             

    domElement= doc.createElement("frame-text-enabled");
    parentElement.appendChild(domElement);
    valueStr.setNum(content->frameTextEnabled());
    text = doc.createTextNode(valueStr);   
    domElement.appendChild(text);             

    domElement= doc.createElement("frame-text");
    parentElement.appendChild(domElement);
    text = doc.createTextNode(content->frameText());   
    domElement.appendChild(text);             


/* FIXME : Save transformations !
    data << transform();
    */
}

void XmlSave::saveImage(PictureContent *imageContent)
{
    // Create parent
    QDomElement imageParent = doc.createElement("image");
    m_imageElements.appendChild(imageParent);

    saveAbstractContent(imageContent, imageParent);

    QDomElement domElement;
    QDomText text;


    // Save image path
    domElement = doc.createElement("path");
    imageParent.appendChild(domElement);
    text = doc.createTextNode(imageContent->getFilePath());   
    domElement.appendChild(text);             

    // Save the effects
    domElement = doc.createElement("effects");
    imageParent.appendChild(domElement);
    QString effectStr;
    foreach (int effect, imageContent->getCPixmap()->getEffects()) {
        // Add each effect using a space as separator
        QString numStr; numStr.setNum(effect);
        effectStr.append(numStr);
        effectStr.append(" ");
    }
    text = doc.createTextNode(effectStr);   
    domElement.appendChild(text);             
}

void XmlSave::saveText(TextContent *textContent)
{
    // Create parent
    QDomElement textParent = doc.createElement("text");
    m_textElements.appendChild(textParent);

    saveAbstractContent(textContent, textParent);

    QDomElement domElement;
    QDomText text;

    // Save item position and size
    domElement= doc.createElement("html-text");
    textParent.appendChild(domElement);
    text = doc.createTextNode(textContent->toHtml());   
    domElement.appendChild(text);             
}

