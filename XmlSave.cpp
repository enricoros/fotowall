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
#include "items/AbstractContent.h"
#include "items/ColorPickerItem.h"
#include "CPixmap.h"
#include "Desk.h"
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
    desk->toXml(m_deskElement);
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
