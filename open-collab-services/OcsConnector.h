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

#ifndef __OcsConnector_h__
#define __OcsConnector_h__

#include <QObject>
#include <QList>
#include <QStringList>
#include <QUrl>
#include "OcsTypes.h"
class QNetworkAccessManager;
class QNetworkReply;

/**
   \brief Experimental Open Collaboration Services connector. Implements a bit of the KNOWLEDGEBASE api.
*/
struct OcsTask;
class OcsConnector : public QObject
{
    Q_OBJECT
    public:
        OcsConnector(QObject * parent);

        void setApiKey(const QString & apiKey);
        QString apikey() const;

        void setServiceUrl(const QUrl & url);
        QUrl serviceUrl() const;

        // knowledgebase
        void kbList(const QString & contentId);

    Q_SIGNALS:
        void kbListed(const KnowledgeItemV1List & items);

    private:
        QNetworkReply * request(const QString & path);
        QNetworkReply * requestPaged(const QString & path, int page = -1);

        QNetworkAccessManager * m_manager;
        QString                 m_apiKey;
        QUrl                    m_serviceUrl;
        QList<OcsTask *>        m_tasks;

    private Q_SLOTS:
        void slotNetworkReply();
};

#endif // OCSCONNECTOR_H
