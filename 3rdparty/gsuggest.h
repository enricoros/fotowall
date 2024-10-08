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
 *      file             : gsuggest.h                                      *
 *      license          : GPL v2 and GPL v3                               *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __GSuggest_h__
#define __GSuggest_h__

#include <QObject>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QTreeWidget>

class GSuggestCompletion : public QObject
{
    Q_OBJECT
    public:
        GSuggestCompletion(QLineEdit *parent = 0);
        ~GSuggestCompletion();

        // ::QObject
        bool eventFilter(QObject *obj, QEvent *ev);

        void showCompletion(const QStringList &choices, const QStringList &hits);

    public slots:
        void doneCompletion();
        void preventSuggest();
        void autoSuggest();
        void handleNetworkData(QNetworkReply *networkReply);

    private:
        QLineEdit *editor;
        QTreeWidget *popup;
        QTimer * timer;
        QNetworkAccessManager networkManager;
};

#endif
