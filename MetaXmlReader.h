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

#ifndef __MetaXmlReader_h__
#define __MetaXmlReader_h__

#include <QList>
#include <QString>
#include <QXmlStreamReader>
class QNetworkAccessManager;

namespace MetaXml {

// structures definition
struct Release {
    QString name;
    QString version;
    QString url;
};

struct Website {
    QString name;
    QString url;
};

/// Reader class
class Reader_1 : public QXmlStreamReader {
    public:
        Reader_1(const QByteArray & data);
        void read();

        // out data
        QList<Release> releases;
        QList<Website> websites;

    private:
        void readReleases();
        Release readRelease();
        void readWebsites();
};

/// Fetcher class
class Connector : public QObject {
    Q_OBJECT
    public:
        static Connector * instance();
        Connector();

        bool hasDone() const;
        bool isValid() const;

        const Reader_1 * reader() const;

    Q_SIGNALS:
        void fetched();
        void fetchError(const QString & description);

    private:
        QNetworkAccessManager * m_nam;
        Reader_1 * m_reader;

    private Q_SLOTS:
        void slotGotReply();
        void slotTimeOut();
};

}

#endif
