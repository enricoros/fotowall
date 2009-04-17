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
#include "PictureContent.h"
#include "TextContent.h"
#include "CPixmap.h"
#include "Desk.h"
#include "items/ColorPickerItem.h"

XmlSave::XmlSave(const QString &filePath)
{
    // Open layout file
    file.setFileName(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        throw 0;
        return;
    }
    out.setDevice(&file);

    // This element contains all the others.
    m_rootElement = doc.createElement("fotowall");
    // This is general informations about the project (title...)
    m_projectElement = doc.createElement("project");
    // All the images will be saved in this element
    m_imageElements = doc.createElement("images");
    // All the texts here
    m_textElements = doc.createElement("texts");
    // Desk informations (background, colors...)
    m_deskElement = doc.createElement("desk");

    // Add elements to the root node (fotowall).
    m_rootElement.appendChild(m_projectElement);
    m_rootElement.appendChild(m_deskElement);
    m_rootElement.appendChild(m_imageElements);
    m_rootElement.appendChild(m_textElements);

    // Add the root (and all the sub-nodes) to the document
    doc.appendChild(m_rootElement);
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

void XmlSave::saveProject(QString title, int mode, const ModeInfo& modeInfo)
{
    // Title element
    QDomElement titleElement = doc.createElement("title");
    m_projectElement.appendChild(titleElement);
    QDomText titleText = doc.createTextNode(title);   
    titleElement.appendChild(titleText);             

    // Mode element
    QDomElement modeElement = doc.createElement("mode"), modeId = doc.createElement("id");
    modeElement.appendChild(modeId);
    QString modeStr; modeStr.setNum(mode);
    QDomText modeText = doc.createTextNode(modeStr);   
    modeId.appendChild(modeText);             
    QSizeF modeSize = modeInfo.realSize();
    if(!modeSize.isEmpty()) { // If it is a mode that requires additionnal saving
        QDomElement modeSizeElement = doc.createElement("size");
        QDomElement wElement= doc.createElement("w");
        modeSizeElement.appendChild(wElement);
        QDomElement hElement= doc.createElement("h");
        modeSizeElement.appendChild(hElement);
        modeElement.appendChild(modeSizeElement);
        QString w, h;
        w.setNum(modeSize.width()); h.setNum(modeSize.height());
        wElement.appendChild(doc.createTextNode(w));             
        hElement.appendChild(doc.createTextNode(h));             
        modeElement.appendChild(modeSizeElement);

        QDomElement dpi = doc.createElement("dpi");
        QString dpiStr; dpiStr.setNum(modeInfo.printDpi());
    }
    m_projectElement.appendChild(modeElement);
}

void XmlSave::saveDesk(const Desk *desk)
{
    // Save background colors
    QColor color; 
    QString r, g, b;
    QDomElement domElement, topColor, bottomColor, 
                redElement = doc.createElement("red"), 
                greenElement = doc.createElement("green"),
                blueElement = doc.createElement("blue"), 
                redElement2 = doc.createElement("red"), 
                greenElement2 = doc.createElement("green"),
                blueElement2 = doc.createElement("blue"),
                rElement = doc.createElement("red"), 
                gElement = doc.createElement("green"),
                bElement = doc.createElement("blue"),
                rElement2 = doc.createElement("red"), 
                gElement2 = doc.createElement("green"),
                bElement2 = doc.createElement("blue");
    domElement = doc.createElement("background-color");

    topColor = doc.createElement("top");
    color = desk->m_grad1ColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    redElement.appendChild(doc.createTextNode(r));
    greenElement.appendChild(doc.createTextNode(g));
    blueElement.appendChild(doc.createTextNode(b));
    topColor.appendChild(redElement); topColor.appendChild(greenElement); topColor.appendChild(blueElement);
    domElement.appendChild(topColor);

    bottomColor = doc.createElement("bottom");
    color = desk->m_grad2ColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    redElement2.appendChild(doc.createTextNode(r));
    greenElement2.appendChild(doc.createTextNode(g));
    blueElement2.appendChild(doc.createTextNode(b));
    bottomColor.appendChild(redElement2); bottomColor.appendChild(greenElement2); bottomColor.appendChild(blueElement2);
    domElement.appendChild(bottomColor);

    m_deskElement.appendChild(domElement);

    QDomElement titleColor = doc.createElement("title-color");
    color = desk->m_titleColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    rElement.appendChild(doc.createTextNode(r));
    gElement.appendChild(doc.createTextNode(g));
    bElement.appendChild(doc.createTextNode(b));
    titleColor.appendChild(rElement); titleColor.appendChild(gElement); titleColor.appendChild(bElement);
    m_deskElement.appendChild(titleColor);

    QDomElement foreColor = doc.createElement("foreground-color");
    color = desk->m_foreColorPicker->color();
    r.setNum(color.red()); g.setNum(color.green()); b.setNum(color.blue());
    rElement2.appendChild(doc.createTextNode(r));
    gElement2.appendChild(doc.createTextNode(g));
    bElement2.appendChild(doc.createTextNode(b));
    foreColor.appendChild(rElement2); foreColor.appendChild(gElement2); foreColor.appendChild(bElement2);
    m_deskElement.appendChild(foreColor);
}

void XmlSave::saveAbstractContent(const AbstractContent *content, QDomElement &parentElement)
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

    if(content->frameTextEnabled()) {
        domElement= doc.createElement("frame-text");
        parentElement.appendChild(domElement);
        text = doc.createTextNode(content->frameText());   
        domElement.appendChild(text);             
    }


    // Save transformations (ie: rotations)
    domElement = doc.createElement("transformation");
    QDomElement xRotationElement= doc.createElement("x-rotation");
    QDomElement yRotationElement= doc.createElement("y-rotation");
    QDomElement zRotationElement = doc.createElement("z-rotation");

    QString z;
    x.setNum(content->m_xRotationAngle); y.setNum(content->m_yRotationAngle);
    z.setNum(content->m_zRotationAngle);
    xRotationElement.appendChild(doc.createTextNode(x));
    yRotationElement.appendChild(doc.createTextNode(y));
    zRotationElement.appendChild(doc.createTextNode(z));

    domElement.appendChild(xRotationElement);
    domElement.appendChild(yRotationElement);
    domElement.appendChild(zRotationElement);
    parentElement.appendChild(domElement);
}

void XmlSave::saveImage(const PictureContent *imageContent)
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

void XmlSave::saveText(const TextContent *textContent)
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

