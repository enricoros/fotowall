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

#ifndef __WordCloudContent_h__
#define __WordCloudContent_h__

#include "AbstractContent.h"
#include "WordCloud/WordCloud.h"
#include <QPixmap>
class Canvas;
class QGraphicsScene;

/**
    \brief Use another Canvas as content
*/
class WordCloudContent : public AbstractContent
{
    Q_OBJECT
    public:
        WordCloudContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
//        ~WordCloudContent();

        WordCloud::Cloud * cloud() const;

        // ::AbstractContent
        QString contentName() const { return tr("WordCloudXXX"); }
        QWidget * createPropertyWidget();
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        void drawContent(QPainter * painter);
        QPixmap renderContent(const QSize & size, Qt::AspectRatioMode ratio) const;

//        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

    private:
        QGraphicsScene * m_cloudScene;
        WordCloud::Cloud * m_cloud;

    private Q_SLOTS:
        void slotRepaintScene(const QList<QRectF> & exposed);
};

#endif
