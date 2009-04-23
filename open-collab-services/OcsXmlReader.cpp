/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "OcsXmlReader.h"
#include <QDebug>
#include <QVariant>

/** KNOWLEDGEBASE SERVICE
Reply to: /v1/knowledgebase/data/40
    <?xml version="1.0"?>
    <ocs>
     <meta>
      <status>ok</status>
      <message></message>
     </meta>
     <data>
      <knowledgebase>
       <id>40</id>
       <status>answered</status>
       <contentid>71320</contentid>
       <user>koral</user>
       <changed>2009-04-18T01:00:12+02:00</changed>
       <name>Features request</name>
       <description>How do I request new features?</description>
       <answer>Just find FotoWall on http://www.kde-apps.org and add a message there, describing the feature.</answer>
       <comments>0</comments>
       <detailpage>http://www.opendesktop.org/content/show.php?action=knowledgebase&amp;content=71320&amp;kbid=40</detailpage>
      </knowledgebase>
     </data>
    </ocs>
*/


OcsXmlReader::OcsXmlReader(const QByteArray & data)
    : QXmlStreamReader(data)
{
}

OcsXmlReader::~OcsXmlReader()
{
    qDeleteAll(m_knowledgeItems);
}

bool OcsXmlReader::read(Flavour flavour)
{
    m_flavour = flavour;

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == "ocs") { // <ocs>
                while (!atEnd()) {
                    readNext();
                    if (isEndElement())
                        break;
                    if (isStartElement()) {
                        if (name() == "meta")
                            readEMeta();
                        else if (name() == "data")
                            readEData();
                        else
                            readEUnknown(true);
                    }
                } // </ocs>
            } else
                raiseError("This is not an OCS xml file");
        }
    }
    if (error()) {
        qWarning("OcsXmlReader::read: error reading type %d, because: %s", flavour, qPrintable(errorString()));
        // TODO clear data
    }
    return !error();
}

QList<KnowledgeItemV1 *> OcsXmlReader::takeKnowledgeItems()
{
    QList<KnowledgeItemV1 *> items = m_knowledgeItems;
    m_knowledgeItems.clear();
    return items;
}

void OcsXmlReader::readEUnknown(bool verbose)
{
    if (verbose)
        qWarning("OcsXmlReader::readEUnknown: entering element '%s'", qPrintable(name().toString()));
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement())
            readEUnknown(verbose);
    }
}

void OcsXmlReader::elementToObject(QObject * object)
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement()) {
            QString pName = name().toString();
            QString pValue = readElementText();
            object->setProperty(qPrintable(pName), pValue);
        }
    }
}

QMap<QString, QString> OcsXmlReader::elementToMap()
{
    QMap<QString, QString> map;
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement())
            map[name().toString()] = readElementText().trimmed();
    }
    return map;
}

void OcsXmlReader::readEMeta()
{
    QMap<QString, QString> map = elementToMap();

    bool ok = !map["status"].compare("ok", Qt::CaseInsensitive);
    QString message = map["message"];
    int itemsCount = map["totalitems"].toInt();
    int itemsPerPage = map["itemsperpage"].toInt();

    if (!ok)
        raiseError("Status is not 'Ok'");
}

void OcsXmlReader::readEData()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement()) {
            if (name() == "knowledgebase" || (m_flavour == KnowledgebaseList && name() == "content"))
                readEKnowledgeItemV1();
            else
                readEUnknown(true);
        }
    }
}

void OcsXmlReader::readEKnowledgeItemV1()
{
    KnowledgeItemV1 * item = new KnowledgeItemV1();
    m_knowledgeItems.append(item);
    elementToObject(item);
}
