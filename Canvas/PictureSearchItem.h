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

#ifndef __PictureSearchItem_h__
#define __PictureSearchItem_h__

#include <QGraphicsProxyWidget>
class AbstractPictureService;
class GSuggestCompletion;
class QLabel;
class QNetworkAccessManager;
class SearchSymbol;
class Ui_PictureSearchItem;

// uncomment following to enable Ariya Hidayat's google suggestions code
//#define ENABLE_GCOMPLETION

class PictureSearchItem : public QGraphicsProxyWidget
{
    Q_OBJECT
    public:
        PictureSearchItem(QNetworkAccessManager * extAccessManager, QGraphicsItem * parent = 0);
        ~PictureSearchItem();

        // used to download pictures
        AbstractPictureService * pictureService() const;

        // ::QGraphicsItem
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        static int LastProvider;

    private:
        QNetworkAccessManager * m_extAccessManager;
        AbstractPictureService * m_pictureService;
#ifdef ENABLE_GCOMPLETION
        GSuggestCompletion * m_completion;
#endif
        Ui_PictureSearchItem * m_ui;
        SearchSymbol * m_searchSymbol;

    private Q_SLOTS:
        void slotProviderChanged();
        void slotSearchClicked();
        void slotSearchBegun();
        void slotSearchResult(int idx, const QString & title, int thumb_w, int thumb_h);
        void slotSearchThumbnail(int idx, const QPixmap & thumbnail);
        void slotSearchEnded(bool error);
};

#endif
