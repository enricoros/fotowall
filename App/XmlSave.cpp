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

#include "App/XmlSave.h"

#include "Canvas/AbstractContent.h"
#include "Canvas/CanvasModeInfo.h"
#include "Canvas/Canvas.h"
#include "App.h"
#include "Settings.h"

#include <QFile>
#include <QList>
#include <QMessageBox>
#include <QTextStream>


bool XmlSave::save(const QString & filePath, const Canvas * canvas, const CanvasModeInfo * modeInfo)
{
    // build up the DOM tree
    XmlSave xmlSave;
    if (modeInfo)
        xmlSave.saveProject(modeInfo);
    if (canvas) {
        xmlSave.saveCanvas(canvas);
        xmlSave.saveContent(canvas);
    }

    // save to disk
    bool saveOk = xmlSave.writeFile(filePath);

    // if saved, add to the recent history
    if (saveOk)
        App::settings->addRecentFotowallUrl(QUrl(filePath));

    // tell about the exit status
    return saveOk;
}

XmlSave::XmlSave()
{
    // This element contains all the others.
    m_rootElement = doc.createElement("fotowall");
    // This is general informations about the project (title...)
    m_projectElement = doc.createElement("project");
    // All the contents will be saved in this element
    m_contentElements = doc.createElement("content");
    // Canvas informations (background, colors...)
    m_canvasElement = doc.createElement("desk");

    // Add elements to the root node (fotowall).
    m_rootElement.appendChild(m_projectElement);
    m_rootElement.appendChild(m_canvasElement);
    m_rootElement.appendChild(m_contentElements);

    // Add the root (and all the sub-nodes) to the document
    doc.appendChild(m_rootElement);
}

bool XmlSave::writeFile(const QString & filePath)
{
    // Open fotowall file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(0, QObject::tr("File Error"), QObject::tr("Error saving to the Fotowall file '%1'").arg(filePath));
        return false;
    }

    // Add at the begining : <?xml version="1.0" ?>
    QDomNode noeud = doc.createProcessingInstruction("xml","version=\"1.0\" ");
    doc.insertBefore(noeud, doc.firstChild());

    // save in the file (4 spaces indent)
    QTextStream out(&file);
    doc.save(out, 4);
    file.close();
    return true;
}

void XmlSave::saveContent(const Canvas * canvas)
{
    foreach (AbstractContent * content, canvas->m_content) {
        QDomElement element = doc.createElement("dummy-renamed-element");
        m_contentElements.appendChild(element);
        content->toXml(element);

        // add a flag to the background element
        if (canvas->m_backContent == content) {
            QDomElement bgEl = doc.createElement("set-as-background");
            element.appendChild(bgEl);
        }
    }
}

void XmlSave::saveCanvas(const Canvas *canvas)
{
    canvas->toXml(m_canvasElement);
}

void XmlSave::saveProject(const CanvasModeInfo * modeInfo)
{
    // Mode element
    QDomElement modeElement = doc.createElement("mode");
    QDomElement modeId = doc.createElement("id");
    modeElement.appendChild(modeId);
    QDomText modeText = doc.createTextNode(QString::number(modeInfo->projectMode()));
    modeId.appendChild(modeText);
    QSizeF modeSize = modeInfo->fixedSizeInches();
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
        QString dpiStr; dpiStr.setNum(modeInfo->printDpi());
    }
    m_projectElement.appendChild(modeElement);
}
