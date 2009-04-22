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
#include <QStringList>
#include <QGraphicsView>
#include <QMessageBox>
#include "Desk.h"
#include "CPixmap.h"
#include "frames/FrameFactory.h"
#include "items/ColorPickerItem.h"
#include "FotoWall.h"


XmlRead::XmlRead(const QString &filePath, Desk *desk) : m_desk(desk)
{
    // Load the file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0, tr("Loading error"), tr("Unable to load the layout file %1").arg(filePath));
        throw(0);
        return;
    }

    // And create the XML document into memory (with nodes...)
    QString *error = new QString();
    if (!doc.setContent(&file, false, error)) {
        QMessageBox::critical(0, tr("Parsing error"), tr("Unable to parse the layout file %1. The error was: %2").arg(filePath, *error));
        file.close();
        throw(0);
        return;
    }
    file.close();

    QDomElement root = doc.documentElement(); // The root node
    m_projectElement = root.firstChildElement("project"); // Get the project node
    m_deskElement = root.firstChildElement("desk");
    // Get the parent images node (containing all the images)
    m_imagesElement = root.firstChildElement("images");
    m_textsElement = root.firstChildElement("texts");

    prepareRestore();
}

XmlRead::~XmlRead()
{
}

void XmlRead::readProject(FotoWall *fotowall)
{
    m_desk->setTitleText(m_projectElement.firstChildElement("title").text());

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

void XmlRead::readDesk()
{
    QDomElement domElement;
    int r, g, b;
    // Load image size saved in the rect node
    domElement = m_deskElement.firstChildElement("background-color").firstChildElement("top");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_desk->m_grad1ColorPicker->setColor(QColor(r, g, b));

    domElement = m_deskElement.firstChildElement("background-color").firstChildElement("bottom");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_desk->m_grad2ColorPicker->setColor(QColor(r, g, b));

    domElement = m_deskElement.firstChildElement("title-color");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_desk->m_titleColorPicker->setColor(QColor(r, g, b));

    domElement = m_deskElement.firstChildElement("foreground-color");
    r = domElement.firstChildElement("red").text().toInt();
    g = domElement.firstChildElement("green").text().toInt();
    b = domElement.firstChildElement("blue").text().toInt();
    m_desk->m_foreColorPicker->setColor(QColor(r, g, b));

    // Show the colors
    m_desk->update();
}

void XmlRead::prepareRestore()
{
    qDeleteAll(m_desk->m_content);
    m_desk->m_content.clear();
    m_desk->m_backContent = 0;
}

void XmlRead::readImages()
{
    // for each '<image>' in '<images>'
    for (QDomElement imageElement = m_imagesElement.firstChildElement("image"); !imageElement.isNull(); imageElement = imageElement.nextSiblingElement("image")) {

        // Create image item (connect slots...)
        PictureContent * content = m_desk->createPicture(QPoint());

        // restore all properties
        content->fromXml(imageElement);
    }
}

void XmlRead::readText()
{
    // for each '<text>' in '<texts>'
    for (QDomElement textE = m_textsElement.firstChildElement("text"); !textE.isNull(); textE = textE.nextSiblingElement("text")) {

        // Create text item (connect slots...)
        TextContent * content = m_desk->createText(QPoint());

        // restore all properties
        content->fromXml(textE);
    }
}
