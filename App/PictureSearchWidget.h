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

#ifndef __PictureSearchWidget_h__
#define __PictureSearchWidget_h__

#include <QWidget>
class GSuggestCompletion;
class QLabel;
class QNetworkAccessManager;
class SearchSymbol;
class Ui_PictureSearchWidget;

// uncomment following to disable Google Images search (Google doesn't offer the API anymore)
//#define ENABLE_GOOGLE_IMAGES_API
// uncomment following to enable Ariya Hidayat's google suggestions code
//#define ENABLE_GCOMPLETION

class PictureSearchWidget : public QWidget
{
    Q_OBJECT
    public:
        PictureSearchWidget(QNetworkAccessManager * extAccessManager, QWidget * parent = 0);
        ~PictureSearchWidget();

        // focus the line edit
        void setFocus();

        // ::QWidget
        void closeEvent(QCloseEvent *event);
        void paintEvent(QPaintEvent * event);

    Q_SIGNALS:
        void requestClosure();

    private:
#ifdef ENABLE_GCOMPLETION
        GSuggestCompletion * m_completion;
#endif
        QNetworkAccessManager * m_extAccessManager;
        Ui_PictureSearchWidget * m_ui;
        SearchSymbol * m_searchSymbol;
        static int LastProvider;

    private Q_SLOTS:
        void slotProviderChanged();
        void slotSearchClicked();
        void slotSearchBegun();
        void slotSearchResult(int idx, const QString & title, int thumb_w, int thumb_h);
        void slotSearchThumbnail(int idx, const QPixmap & thumbnail);
        void slotSearchEnded(bool error);
};

#endif
