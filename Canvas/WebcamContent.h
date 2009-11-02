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

#ifndef __WebcamContent_h__
#define __WebcamContent_h__

#include "AbstractContent.h"
#include <QPixmap>
class QSvgRenderer;

/**
    \brief Displays live video from a WebCam
*/
class WebcamContent : public AbstractContent
{
    Q_OBJECT
    public:
        WebcamContent(int input, QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~WebcamContent();

        // ::AbstractContent
        QString contentName() const { return tr("Webcam"); }
        bool fromXml(QDomElement & contentElement);
        void toXml(QDomElement & contentElement) const;
        void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio);
        QPixmap toPixmap(const QSize & size, Qt::AspectRatioMode ratio);
        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

    public Q_SLOTS:
        void setPixmap(const QPixmap & pixmap);

    private:
        int m_input;
        bool m_still;
        QPixmap m_pixmap;
        QPixmap m_dummyPixmap;
        QSvgRenderer * m_dummyRenderer;
        bool m_broken;

    private Q_SLOTS:
        void slotToggleStill();
        void slotToggleSwap();
};

#endif
