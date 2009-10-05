/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __AbstractPictureService_h__
#define __AbstractPictureService_h__

#include <QObject>
#include <QNetworkRequest>
#include <QPixmap>
#include <QUrl>
class QNetworkAccessManager;
class QNetworkReply;

/// \brief Base class for accessing Online picture services
class AbstractPictureService : public QObject
{
    Q_OBJECT
    public:
        // construct the object using a shared networkaccessmanager
        AbstractPictureService(QNetworkAccessManager * manager, QObject * parent = 0);

        // search pictures
        virtual void searchPics(const QString & text) = 0;
        virtual void dropSearch() = 0;

        // describe a search result
        virtual bool imageInfo(int idx, QString * url, QString * title, int * width, int * height) = 0;

        // download a picture
        virtual QNetworkReply * download(int idx) = 0;
        virtual void startPrefetch(int idx) = 0;
        virtual void stopPrefetch(int idx) = 0;

    Q_SIGNALS:
        // signals are emitted in the following order
        void searchStarted();
        void searchResult(int idx, const QString & text, int thumb_width, int thumb_height);
        void searchThumbnail(int idx, const QPixmap & icon);
        void searchEnded(bool error);

    protected:
        // will be used by subclasses to access the network
        QNetworkReply * get(const QNetworkRequest & request);
        QNetworkReply * get(const QUrl & url);

    private:
        AbstractPictureService();
        QNetworkAccessManager * m_nam;
};

#endif
