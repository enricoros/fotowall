/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FlickrInterface_h__
#define __FlickrInterface_h__

#include <QObject>
#include <QMap>
#include <QList>
#include <QNetworkAccessManager>
#include <QPair>
#include <QPixmap>
#include <QUrl>
namespace Internal {
    struct Photo;
};

class FlickrInterface : public QObject
{
    Q_OBJECT
    public:
        FlickrInterface(QObject * parent = 0);
        ~FlickrInterface();

        void searchPics(const QString & text);
        void dropSearch();

        bool imageInfo(int idx, QString * title, int * width, int * height);

        QNetworkReply * download(int idx);
        void startPrefetch(int idx);
        void stopPrefetch(int idx);

    Q_SIGNALS:
        void searchStarted();
        void searchResult(int idx, const QString & text, int thumb_width, int thumb_height);
        void searchThumbnail(int idx, const QPixmap & icon);
        void searchEnded();

    private Q_SLOTS:
        void slotSearchJobFinished();
        void slotThumbJobFinished();

    private:
        bool startNextThumbnailJobs(int count = 1);

        typedef QPair<QString, QString> KeyValue;
        typedef QList<KeyValue> KeyValueList;

        QNetworkReply * sendRequest(const QString & method, const KeyValueList & params);
        QNetworkReply * sendRequestURL(const QUrl & url);

        QString m_apiKey;
        QNetworkAccessManager * m_nam;
        QNetworkReply * m_searchJob;
        QList<Internal::Photo *> m_searchResults;
        QMap<int, QNetworkReply *> m_prefetches;
};

#endif
