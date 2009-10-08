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

#ifndef __GoogleImagesPictureService_h__
#define __GoogleImagesPictureService_h__

#include "AbstractPictureService.h"
#include <QMap>
#include <QList>
#include <QPair>
#include <QUrl>
namespace GoogleImagesInternal {
    struct Photo;
};

class GoogleImagesPictureService : public AbstractPictureService
{
    Q_OBJECT
    public:
        GoogleImagesPictureService(QNetworkAccessManager * manager, QObject * parent = 0);
        ~GoogleImagesPictureService();

        // simple configuration interface. this should be queried and set dinammically
        void configure(int contentType, int sizeType);

        // ::AbstractPictureService
        void searchPics(const QString & text);
        void dropSearch();
        bool imageInfo(int idx, QString * url, QString * title, int * width, int * height);
        QNetworkReply * download(int idx);
        void startPrefetch(int idx);
        void stopPrefetch(int idx);

    private:
        bool parseGoogleSearchReply(const QByteArray &);
        bool startNextThumbnailJobs(int count = 1);

        int m_contentType;
        int m_sizeType;

        QRegExp m_rxHref;
        QRegExp m_rxData;
        QRegExp m_rxStart;
        QRegExp m_rxOther;

        QNetworkReply * m_searchJob;
        QList<GoogleImagesInternal::Photo *> m_searchResults;
        QMap<int, QNetworkReply *> m_prefetches;

    private Q_SLOTS:
        void slotSearchJobFinished();
        void slotThumbJobFinished();
};

#endif
