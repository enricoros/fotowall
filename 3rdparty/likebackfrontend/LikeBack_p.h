/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      file             : likeback_p.h                                    *
 *      license          : GPL v2+                                         *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
                              likeback_p.h
                             -------------------
    begin                : unknown
    imported to LB svn   : 3 june, 2009
    copyright            : (C) 2006 by Sebastien Laout
                           (C) 2008-2009 by Valerio Pilo, Sjors Gielen
    email                : sjors@kmess.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LikeBack_p_h__
#define __LikeBack_p_h__

#include "LikeBack.h"
#include <QNetworkAccessManager>

class LikeBackPrivate
{
    public:
        LikeBackPrivate();
        ~LikeBackPrivate();

        LikeBack::Button         buttons;
        QString                  hostName;
        QString                  remotePath;
        quint16                  hostPort;
        QStringList              acceptedLocales;
        LikeBack::WindowListing  windowListing;
        bool                     showBarByDefault;
        bool                     showBar;
        int                      disabledCount;
        QString                  fetchedEmail;
        QNetworkAccessManager *  nam;
};



// Constructor
LikeBackPrivate::LikeBackPrivate()
  : buttons(LikeBack::DefaultButtons)
  , hostName()
  , remotePath()
  , hostPort(80)
  , acceptedLocales()
  , windowListing(LikeBack::NoListing)
  , showBarByDefault(false)
  , showBar(false)
  , disabledCount(0)
  , fetchedEmail()
  , nam(new QNetworkAccessManager)
{
}



// Destructor
LikeBackPrivate::~LikeBackPrivate()
{
    delete nam;
}


#endif // __LikeBack_p_h__
