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

#ifndef __FlickrPictureService_h__
#define __FlickrPictureService_h__

#include "AbstractPictureService.h"
#include <QList>
#include <QMap>
#include <QPair>
#include <QUrl>
namespace FlickrInternal
{
struct Photo;
};

class FlickrPictureService : public AbstractPictureService
{
  Q_OBJECT
public:
  FlickrPictureService(const QString & apiKey, QNetworkAccessManager * manager, QObject * parent = 0);
  ~FlickrPictureService();

  // ::AbstractPictureService
  void searchPics(const QString & text);
  void dropSearch();
  bool imageInfo(int idx, QString * url, QString * title, int * width, int * height);
  QNetworkReply * download(int idx);
  void startPrefetch(int idx);
  void stopPrefetch(int idx);

private:
  bool startNextThumbnailJobs(int count = 1);

  typedef QPair<QString, QString> KeyValue;
  typedef QList<KeyValue> KeyValueList;

  QNetworkReply * flickrApiCall(const QString & method, const KeyValueList & params);

  QString m_apiKey;
  QNetworkReply * m_searchJob;
  QList<FlickrInternal::Photo *> m_searchResults;
  QMap<int, QNetworkReply *> m_prefetches;

private Q_SLOTS:
  void slotSearchJobFinished();
  void slotThumbJobFinished();
};

#endif
