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

#include "OcsConnector.h"
#include "OcsXmlReader.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

struct OcsTask {
    enum ServiceType {
        ST_Knowledgebase,
    };

    ServiceType serviceType;
    QList<QString> queries;
    QList<QNetworkReply *> pendingReplies;

};

OcsConnector::OcsConnector(QObject * parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

void OcsConnector::setApiKey(const QString & apiKey)
{
    m_apiKey = apiKey;
    if (m_serviceUrl.isValid() && !m_apiKey.isEmpty())
        m_serviceUrl.setUserName(m_apiKey);
}

QString OcsConnector::apikey() const
{
    return m_apiKey;
}

void OcsConnector::setServiceUrl(const QUrl & url)
{
    if (!url.isValid()) {
        qWarning("OcsConnector::setServiceUrl: invalid URL %s", qPrintable(url.toString()));
        return;
    }
    m_serviceUrl = url;
    if (!m_apiKey.isEmpty())
        m_serviceUrl.setUserName(m_apiKey);
}

QUrl OcsConnector::serviceUrl() const
{
    return m_serviceUrl;
}

QNetworkReply * OcsConnector::request(const QString & path)
{
    QString url = m_serviceUrl.toString() + path;
    //qWarning() << url;
    return m_manager->get(QNetworkRequest(QUrl(url)));
}

QNetworkReply * OcsConnector::requestPaged(const QString & path, int page)
{
    return request(page == -1 ? path : (path + "&page=" + QString::number(page)));
}

void OcsConnector::kbList(const QString & contentId)
{
    QString query = "/v1/knowledgebase/data?content=" + contentId + "&sortmode=new";

    OcsTask * task = new OcsTask();
    task->serviceType = OcsTask::ST_Knowledgebase;
    task->queries.append(query);
    QNetworkReply * reply = requestPaged(query);
    reply->setProperty("contentid", contentId);
    connect(reply, SIGNAL(finished()), this, SLOT(slotNetworkReply()));
    task->pendingReplies.append(reply);
}

void OcsConnector::slotNetworkReply()
{
    QNetworkReply * reply = dynamic_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    // handle errors in replies
    if (reply->error() != QNetworkReply::NoError) {
        // TODO handle error in task
        reply->deleteLater();
        qWarning("OcsConnector::slotNetworkReply: error getting '%s'", qPrintable(reply->url().toString()));
        return;
    }

    // get the reply HARDCODED - FIXME
    QByteArray content = reply->readAll();
    OcsXmlReader reader(content);
    reader.read(OcsXmlReader::KnowledgebaseList);

    // got knowledge items
    QString contentid = reply->property("contentid").toString();
    KnowledgeItemV1List allKi = reader.takeKnowledgeItems(), goodKi;
    foreach (KnowledgeItemV1 * item, allKi)
        if (item->contentid() == contentid)
            goodKi.append(item);
    if (!goodKi.isEmpty())
        emit kbListed(goodKi);
    qDeleteAll(allKi);
    allKi.clear();
}
