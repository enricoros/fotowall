/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __OnlineServices_h__
#define __OnlineServices_h__

#include <QList>
#include <QObject>
class QNetworkAccessManager;

// uncomment the following to stop the Updates checking
#define NO_UPDATE_CHECK

class OnlineServices : public QObject
{
  Q_OBJECT
public:
  OnlineServices(QNetworkAccessManager *, QObject * parent = 0);
  ~OnlineServices();

  // return true if already present, otherwise start a search
  bool checkForTutorial();

public Q_SLOTS:
  void openWebpage();
  void openBlog();
  void openYouTube();
  void openTutorial();
#if !defined(NO_UPDATE_CHECK)
  void checkForUpdates();
#endif

Q_SIGNALS:
  void tutorialFound(bool);

private:
#if !defined(NO_UPDATE_CHECK)
  void autoUpdate();
#endif

  enum PostFetchOp
  {
    OpenWebsite,
    OpenBlog
  };
  QNetworkAccessManager * m_nam;
  bool m_haveTutorial;
  QList<PostFetchOp> m_postOps;

private Q_SLOTS:
  void slotCheckTutorialReply();
  void slotConnectorFinished();
};

#endif
