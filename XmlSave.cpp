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
    m_rootElement = doc.createElement("fotowall");
    m_imageElements = doc.createElement("images");
    m_textElements = doc.createElement("texts");
    m_projectElement = doc.createElement("project");
    m_rootElement.appendChild(m_projectElement);
    m_rootElement.appendChild(m_imageElements);
    m_rootElement.appendChild(m_textElements);


    doc.appendChild(m_rootElement);

    file.setFileName(filePath);
    if (!file.open(QIODevice::WriteOnly))     //ouverture du fichier de sauvegarde
        return;                              //en ecriture
    out.setDevice(&file);                     //association du flux au fichier
}

XmlSave::~XmlSave()
{
   //insertion en début de document de <?xml version="1.0" ?>
   QDomNode noeud = doc.createProcessingInstruction("xml","version=\"1.0\" ");
   doc.insertBefore(noeud,doc.firstChild());
   //sauvegarde dans le flux (4 espaces de décalage dans l’arborescence)
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
    domElement= doc.createElement("rect");
    parentElement.appendChild(domElement);
    QRectF rect = content->boundingRect();
    QString x, y, w, h;
    x.setNum(rect.x()); y.setNum(rect.y());
    w.setNum(rect.width()); h.setNum(rect.height());
    text = doc.createTextNode(x+" "+y+" "+w+" "+h);   
    domElement.appendChild(text);             

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
    domElement= doc.createElement("frame");
    parentElement.appendChild(domElement);
    text = doc.createTextNode(valueStr);   
    domElement.appendChild(text);             

    domElement= doc.createElement("frame-text-enabled");
    parentElement.appendChild(domElement);
    valueStr.setNum(content->frameTextEnabled());
    text = doc.createTextNode(valueStr);   
    domElement.appendChild(text);             

    domElement= doc.createElement("text");
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

    //// Save the effects
    QDomElement effectElement = doc.createElement("effects");
    imageParent.appendChild(effectElement);
    QString effectStr;
    foreach (int effect, imageContent->getCPixmap()->getEffects()) {
        // Add each effect using a space as separator
        QString numStr; numStr.setNum(effect);
        effectStr.append(numStr);
        effectStr.append(" ");
    }
    QDomText effectText = doc.createTextNode(effectStr);   
    effectElement.appendChild(effectText);             
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

