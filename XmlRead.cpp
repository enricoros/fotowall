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
#include "frames/FrameFactory.h"
#include "items/AbstractContent.h"
#include "items/ColorPickerItem.h"
#include "items/PictureContent.h"
#include "items/TextContent.h"
#include "items/VideoContent.h"
#include "CPixmap.h"
#include "Desk.h"
#include "FotoWall.h"
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QStringList>


XmlRead::XmlRead(const QString & filePath)
{
    // Load the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, tr("Loading error"), tr("Unable to load the FotoWall file %1").arg(filePath));
        throw(0);
        return;
    }

    // And create the XML document into memory (with nodes...)
    QString *error = new QString();
    QDomDocument doc;
    if (!doc.setContent(&file, false, error)) {
        QMessageBox::critical(0, tr("Parsing error"), tr("Unable to parse the FotoWall file %1. The error was: %2").arg(filePath, *error));
        file.close();
        throw(0);
        return;
    }
    file.close();

    QDomElement root = doc.documentElement(); // The root node
    m_projectElement = root.firstChildElement("project"); // Get the project node
    m_deskElement = root.firstChildElement("desk");
    m_contentElement = root.firstChildElement("content");
}

void XmlRead::readProject(FotoWall *fotowall)
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
        fotowall->setModeInfo(modeInfo);
    }

    int mode = modeElement.firstChildElement("id").text().toInt();
    fotowall->restoreMode(mode);
}

void XmlRead::readDesk(Desk * desk)
{
    // clear Desk
    qDeleteAll(desk->m_content);
    desk->m_content.clear();

    desk->fromXml(m_deskElement);
}

void XmlRead::readContent(Desk * desk)
{
    // for each child of 'content'
    for (QDomElement element = m_contentElement.firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {

        // create the right kind of content
        AbstractContent * content = 0;
        if (element.tagName() == "picture")
            content = desk->createPicture(QPoint());
        else if (element.tagName() == "text")
            content = desk->createText(QPoint());
        else if (element.tagName() == "video")
            content = desk->createVideo(element.attribute("input").toInt(), QPoint());
        if (!content) {
            qWarning("XmlRead::readContent: unknown content type '%s'", qPrintable(element.tagName()));
            continue;
        }

        // restore the item, and delete it if something goes wrong
        if (!content->fromXml(element)) {
            desk->m_content.removeAll(content);
            delete content;
        }
    }
}
