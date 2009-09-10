/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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

#include "MetaXmlReader.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

#define METAXML_BASE_URL "http://www.enricoros.com/opensource/fotowall/.meta"
#define NETWORK_TIMEOUT 10000

/* pre-0.8.0 used this settings:
    #define OLD_CHECK_URL "http://code.google.com/p/fotowall/"
    #define OLD_MAGIC_TOKEN "Last version is "
    #define OLD_DOWNLOADS_URL "http://code.google.com/p/fotowall/downloads/list"
*/

using namespace MetaXml;

Reader_1::Reader_1(const QByteArray & data)
  : QXmlStreamReader(data)
{
    read();
}

void Reader_1::read()
{
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement()) {
            if (name() == QLatin1String("fotowall-meta"))
                continue;
            else if (name() == QLatin1String("releases"))
                readReleases();
            else if (name() == QLatin1String("websites"))
                readWebsites();
            else
                readElementText();
        }
    }
}

void Reader_1::readReleases()
{
    releases.clear();
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement()) {
            if (name() == QLatin1String("release"))
                releases.append(readRelease());
            else
                readElementText();
        }
    }
}

Release Reader_1::readRelease()
{
    Release r;
    r.name = attributes().value("name").toString();
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement()) {
            if (name() == QLatin1String("version"))
                r.version = readElementText();
            else if (name() == QLatin1String("download-url"))
                r.url = readElementText();
            else
                readElementText();
        }
    }
    return r;
}

void Reader_1::readWebsites()
{
    websites.clear();
    while (!atEnd()) {
        readNext();
        if (isEndElement())
            break;
        if (isStartElement()) {
            if (name() == QLatin1String("homepage") || name() == QLatin1String("site")) {
                Website w;
                w.name = attributes().value("name").toString();
                w.url = readElementText();
                if (name() == QLatin1String("homepage"))
                    websites.prepend(w);
                else
                    websites.append(w);
            } else
                readElementText();
        }
    }
}

Fetcher_1::Fetcher_1(QObject * parent)
  : QObject(parent)
  , m_nam(new QNetworkAccessManager)
  , m_reader(0)
{
    QNetworkRequest request(QUrl(METAXML_BASE_URL));
    QNetworkReply * reply = m_nam->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(slotGotReply()));
    QTimer::singleShot(NETWORK_TIMEOUT, this, SLOT(slotTimeOut()));
}

const Reader_1 * Fetcher_1::reader() const
{
    return m_reader;
}

void Fetcher_1::slotGotReply()
{
    if (!m_nam)
        return;
    QNetworkReply * reply = static_cast<QNetworkReply *>(sender());
    QByteArray replyData = reply->readAll();
    QNetworkReply::NetworkError error = reply->error();
    reply->deleteLater();
    if (error != QNetworkReply::NoError) {
        emit fetchError(tr("Network Error"));
        return;
    }

    // read the data and notify the completion
    delete m_reader;
    m_reader = new Reader_1(replyData);
    emit fetched();
}

void Fetcher_1::slotTimeOut()
{
    if (m_nam) {
        m_nam->deleteLater();
        m_nam = 0;
    }
    emit fetchError(tr("Network Timeout"));
}
