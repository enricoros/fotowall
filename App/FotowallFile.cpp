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

#include "FotowallFile.h"

#include "Canvas/AbstractContent.h"
#include "Canvas/CanvasModeInfo.h"
#include "Canvas/Canvas.h"
#include "Canvas/PictureContent.h"
#include "Canvas/TextContent.h"
#include "Canvas/WebcamContent.h"
#include "App.h"
#include "Settings.h"

#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QString>
#include <QTextStream>

/** \brief XML File structure

  Old (0.4 to 0.8.96)
    <fotowall>
      <project>
        <mode />
      </project>
      <desk />
      <content>
        <type .. />
        ...
        <type .. />
      </content>
    </fotowall>

  Since Fotowall 0.8.97
    <fotowall format="2" version="0.8.97">
      <canvas>
        <mode />
        <background />
        <content />
      </canvas>
    </fotowall>

**/

bool FotowallFile::read(const QString & filePath, Canvas * canvas)
{
    // load the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, QObject::tr("Loading error"), QObject::tr("Unable to load the Fotowall file %1").arg(filePath));
        return false;
    }

    // load the DOM
    QString error;
    QDomDocument doc;
    if (!doc.setContent(&file, false, &error)) {
        QMessageBox::critical(0, QObject::tr("Parsing error"), QObject::tr("Unable to parse the Fotowall file %1. The error was: %2").arg(filePath, error));
        return false;
    }
    file.close();

    // get the main nodes
    QDomElement root = doc.documentElement();
    QDomElement projectModeElement = root.firstChildElement("project").firstChildElement("mode");
    QDomElement canvasElement = root.firstChildElement("desk");
    QDomElement contentElement = root.firstChildElement("content");

    // 1. read canvas background properties
    canvas->fromXml(canvasElement);

    // 2. read and set canvas modeInfo
    CanvasModeInfo * modeInfo = new CanvasModeInfo();
    modeInfo->fromXml(projectModeElement);
    canvas->setModeInfo(modeInfo);

    // 3. read and create contents
    readContent(canvas, contentElement);

    // add to the recent history
    App::settings->addRecentFotowallUrl(QUrl(filePath));
    return true;
}

void FotowallFile::readContent(Canvas * canvas, QDomElement & parentElement)
{
    // for each child of 'content'
    for (QDomElement element = parentElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {

        // create the right kind of content
        AbstractContent * content = 0;
        if (element.tagName() == "picture")
            content = canvas->createPicture(QPoint());
        else if (element.tagName() == "text")
            content = canvas->createText(QPoint());
        else if (element.tagName() == "webcam")
            content = canvas->createWebcam(element.attribute("input").toInt(), QPoint());
        if (!content) {
            qWarning("FotowallFile::readContent: unknown content type '%s'", qPrintable(element.tagName()));
            continue;
        }

        // restore the item, and delete it if something goes wrong
        if (!content->fromXml(element)) {
            canvas->deleteContent(content);
            continue;
        }

        // restore the background element of the canvas
        if (element.firstChildElement("set-as-background").isElement()) {
            if (canvas->m_backContent) {
                qWarning("FotowallFile::readContent: only 1 element with <set-as-background/> allowed");
                continue;
            }
            canvas->setBackContent(content);
        }
    }
}


bool FotowallFile::save(const QString & filePath, const Canvas * canvas)
{
    FotowallFile file;
    file.saveProject(canvas->modeInfo());
    file.saveCanvas(canvas);
    file.saveContent(canvas);

    // save to disk
    bool saveOk = file.writeFile(filePath);

    // if saved, add to the recent history
    if (saveOk)
        App::settings->addRecentFotowallUrl(QUrl(filePath));

    // tell about the exit status
    return saveOk;
}

void FotowallFile::saveContent(const Canvas * canvas)
{
    foreach (AbstractContent * content, canvas->m_content) {
        QDomElement element = doc.createElement("dummy-renamed-element");
        m_contentElement.appendChild(element);
        content->toXml(element);

        // add a flag to the background element
        if (canvas->m_backContent == content) {
            QDomElement bgEl = doc.createElement("set-as-background");
            element.appendChild(bgEl);
        }
    }
}

void FotowallFile::saveCanvas(const Canvas *canvas)
{
    canvas->toXml(m_canvasElement);
}

void FotowallFile::saveProject(const CanvasModeInfo * modeInfo)
{
    // ### MOVE OUTTA HERE

    // This element contains all the others.
    m_rootElement = doc.createElement("fotowall");
    // This is general informations about the project (title...)
    m_projectElement = doc.createElement("project");
    QDomElement projectModeElement = doc.createElement("mode");
    m_projectElement.appendChild(projectModeElement);
    // All the contents will be saved in this element
    m_contentElement = doc.createElement("content");
    // Canvas informations (background, colors...)
    m_canvasElement = doc.createElement("desk");

    // Add elements to the root node (fotowall).
    m_rootElement.appendChild(m_projectElement);
    m_rootElement.appendChild(m_canvasElement);
    m_rootElement.appendChild(m_contentElement);

    // Add the root (and all the sub-nodes) to the document
    doc.appendChild(m_rootElement);

    // Mode element
    modeInfo->toXml(projectModeElement);
}

bool FotowallFile::writeFile(const QString & filePath)
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
