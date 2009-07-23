/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
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

#ifndef __WebContentSelectorItem_h__
#define __WebContentSelectorItem_h__

#include <QGraphicsWidget>
class FlickrInterface;
class Frame;
class GSuggestCompletion;
class QLabel;
class QNetworkAccessManager;
class SearchSymbol;
class Ui_WebContentSelectorItem;

//#define ENABLE_GCOMPLETION

class WebContentSelectorItem : public QGraphicsWidget
{
    Q_OBJECT
    public:
        WebContentSelectorItem(QNetworkAccessManager * deskAccessManager, QGraphicsItem * parent = 0);
        ~WebContentSelectorItem();

        // used by Desk to download pictures
        FlickrInterface * flickrInterface() const;

        // ::QGraphicsItem
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    private:
        QNetworkAccessManager * m_deskAccessManager;
        Frame * m_frame;
        FlickrInterface * m_flickr;
#ifdef ENABLE_GCOMPLETION
        GSuggestCompletion * m_completion;
#endif
        Ui_WebContentSelectorItem * m_ui;
        SearchSymbol * m_searchSymbol;

    private Q_SLOTS:
        void slotSearchClicked();
        void slotSearchBegun();
        void slotSearchResult(int idx, const QString & title, int thumb_w, int thumb_h);
        void slotSearchThumbnail(int idx, const QPixmap & thumbnail);
        void slotSearchEnded();
};

#endif
