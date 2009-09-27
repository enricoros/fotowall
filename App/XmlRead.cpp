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

#include "Canvas/AbstractContent.h"
#include "Canvas/Canvas.h"
#include "Canvas/PictureContent.h"
#include "Canvas/TextContent.h"
#include "Canvas/WebcamContent.h"
#include "Frames/FrameFactory.h"
#include "App.h"
#include "MainWindow.h"
#include "Settings.h"

#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QStringList>


bool XmlRead::read(const QString & filePath, MainWindow * mw, Canvas * canvas)
{
    // parse the DOM of the file
    XmlRead xmlRead;
    if (!xmlRead.loadFile(filePath))
        return false;

    // create objects and read data
    if (mw)
        xmlRead.readProject(mw);
    if (canvas) {
        xmlRead.readCanvas(canvas);
        xmlRead.readContent(canvas);
    }

    // add to the recent history
    App::settings->addRecentFotowallUrl(QUrl(filePath));
    return true;
}

bool XmlRead::loadFile(const QString & filePath)
{
    // Load the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, QObject::tr("Loading error"), QObject::tr("Unable to load the Fotowall file %1").arg(filePath));
        return false;
    }

    // And create the XML document into memory (with nodes...)
    QString error;
    QDomDocument doc;
    if (!doc.setContent(&file, false, &error)) {
        QMessageBox::critical(0, QObject::tr("Parsing error"), QObject::tr("Unable to parse the Fotowall file %1. The error was: %2").arg(filePath, error));
        return false;
    }
    file.close();

    // Get the 3 main Nodes
    QDomElement root = doc.documentElement(); // the root node
    m_projectElement = root.firstChildElement("project");
    m_canvasElement = root.firstChildElement("desk");
    m_contentElement = root.firstChildElement("content");
    return true;
}

void XmlRead::readProject(MainWindow *mainWindow)
{
    ModeInfo modeInfo;
    QDomElement modeElement = m_projectElement.firstChildElement("mode");
    QDomElement sizeElement = modeElement.firstChildElement("size");
    if (!sizeElement.isNull()) {
        float w = sizeElement.firstChildElement("w").text().toFloat();
        float h = sizeElement.firstChildElement("h").text().toFloat();
        modeInfo.setRealSizeInches(w, h);

    }
    QDomElement dpiElement = sizeElement.firstChildElement("dpi");
    if(!dpiElement.isNull()) {
        int dpi = dpiElement.text().toInt();
        modeInfo.setPrintDpi(dpi);
    }

    int mode = modeElement.firstChildElement("id").text().toInt();
    mainWindow->setModeInfo(modeInfo);
    mainWindow->restoreMode(mode);
}

void XmlRead::readCanvas(Canvas * canvas)
{
    // clear Canvas [TODO: clear every content! or disasters happen]
    qDeleteAll(canvas->m_content);
    canvas->m_content.clear();
    canvas->m_configs.clear();
    canvas->m_backContent = 0;

    canvas->fromXml(m_canvasElement);
}

void XmlRead::readContent(Canvas * canvas)
{
    // for each child of 'content'
    for (QDomElement element = m_contentElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {

        // create the right kind of content
        AbstractContent * content = 0;
        if (element.tagName() == "picture")
            content = canvas->createPicture(QPoint());
        else if (element.tagName() == "text")
            content = canvas->createText(QPoint());
        else if (element.tagName() == "webcam")
            content = canvas->createWebcam(element.attribute("input").toInt(), QPoint());
        if (!content) {
            qWarning("XmlRead::readContent: unknown content type '%s'", qPrintable(element.tagName()));
            continue;
        }

        // restore the item, and delete it if something goes wrong
        if (!content->fromXml(element)) {
            canvas->m_content.removeAll(content);
            delete content;
            continue;
        }

        // restore the background element of the canvas
        if (element.firstChildElement("set-as-background").isElement()) {
            if (canvas->m_backContent) {
                qWarning("XmlRead::readContent: only 1 element with <set-as-background/> allowed");
                continue;
            }
            canvas->setBackContent(content);
        }
    }
}
