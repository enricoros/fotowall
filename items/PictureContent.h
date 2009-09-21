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

#ifndef __PictureContent_h__
#define __PictureContent_h__

#include "AbstractContent.h"
#include "PictureEffect.h"
class CPixmap;
class QNetworkReply;

/**
    \brief Transformable picture, with lots of gadgets
*/
class PictureContent : public AbstractContent
{
    Q_OBJECT
    public:
        PictureContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~PictureContent();

        bool loadPhoto(const QString & fileName, bool keepRatio = false, bool setName = false);
        bool loadFromNetwork(const QString & url, QNetworkReply * reply = 0, const QString & title = QString(), int width = -1, int height = -1);
        void addEffect(const PictureEffect & effect);

        // ::AbstractContent
        QString contentName() const { return tr("Picture"); }
        QWidget * createPropertyWidget();
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        QPixmap renderContent(const QSize & size, Qt::AspectRatioMode ratio) const;
        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void dropEvent(QGraphicsSceneDragDropEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

        CPixmap getPhoto() const;

    Q_SIGNALS:
        void flipHorizontally();
        void flipVertically();
        void crop();

    private:
        void dropNetworkConnection();
        void applyPostLoadEffects();
        QString     m_fileUrl;
        CPixmap *   m_photo;
        QPixmap     m_cachedPhoto;
        bool        m_opaquePhoto;
        double      m_progress;
        int         m_netWidth;
        int         m_netHeight;
        QNetworkReply * m_netReply;
        QList<PictureEffect> m_afterLoadEffects;

    private Q_SLOTS:
        bool slotLoadNetworkData();
        void slotNetworkError();
        void slotNetworkProgress(qint64, qint64);
};

#endif
