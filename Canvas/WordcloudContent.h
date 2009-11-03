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

#ifndef __WordcloudContent_h__
#define __WordcloudContent_h__

#include "AbstractContent.h"
#include "Shared/AbstractResourceProvider.h"
#include "Wordcloud/Cloud.h"
#include <QPixmap>
class Canvas;
class QGraphicsScene;

/**
    \brief Use another Canvas as content
*/
class WordcloudContent : public AbstractContent, public SingleResourceLoaner
{
    Q_OBJECT
    public:
        WordcloudContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~WordcloudContent();

        // ::AbstractContent
        QString contentName() const { return tr("Wordcloud"); }
        bool fromXml(QDomElement & contentElement);
        void toXml(QDomElement & contentElement) const;
        void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio);

        // ::SingleResourceLoaner
        QVariant takeResource();
        void returnResource(const QVariant &);

    private:
        QGraphicsScene * m_cloudScene;
        Wordcloud::Cloud * m_cloud;
        bool m_cloudTaken;

    private Q_SLOTS:
        void slotRepaintScene(const QList<QRectF> & exposed);
};

#endif
