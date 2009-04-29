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
#include "CPixmap.h"
#include "Desk.h"
#include "PictureContent.h"
#include "TextContent.h"
#include "items/ColorPickerItem.h"
#include <QMessageBox>
#include <QFile>

XmlSave::XmlSave(const QString &filePath)
{
    // Open fotowall file
    file.setFileName(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(0, tr("File Error"), tr("Error saving to the FotoWall file '%1'").arg(filePath));
        throw 0;
        return;
    }
    out.setDevice(&file);

    // This element contains all the others.
    m_rootElement = doc.createElement("fotowall");
    // This is general informations about the project (title...)
    m_projectElement = doc.createElement("project");
    // All the contents will be saved in this element
    m_contentElements = doc.createElement("content");
    // Desk informations (background, colors...)
    m_deskElement = doc.createElement("desk");

    // Add elements to the root node (fotowall).
    m_rootElement.appendChild(m_projectElement);
    m_rootElement.appendChild(m_deskElement);
    m_rootElement.appendChild(m_contentElements);

    // Add the root (and all the sub-nodes) to the document
    doc.appendChild(m_rootElement);
}

XmlSave::~XmlSave()
{
   //Add at the begining : <?xml version="1.0" ?>
   QDomNode noeud = doc.createProcessingInstruction("xml","version=\"1.0\" ");
   doc.insertBefore(noeud,doc.firstChild());
   //save in the file (4 spaces indent)
   doc.save(out, 4);
   file.close();
}

void XmlSave::saveContent(const Desk * desk)
{
    foreach (AbstractContent * content, desk->m_content) {
        QDomElement element = doc.createElement("renamed-element");
        m_contentElements.appendChild(element);
        content->toXml(element);
    }
}

void XmlSave::saveDesk(const Desk *desk)
{
    // Save Title
    QDomElement titleElement = doc.createElement("title");
    m_deskElement.appendChild(titleElement);
    QDomText titleText = doc.createTextNode(desk->titleText());
    titleElement.appendChild(titleText);

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

void XmlSave::saveProject(int mode, const ModeInfo& modeInfo)
{
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
