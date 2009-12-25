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

#ifndef __CanvasViewContent_h__
#define __CanvasViewContent_h__

#include "AbstractContent.h"
#include "Shared/AbstractResourceProvider.h"
#include <QPixmap>
class Canvas;

/**
    \brief Use another Canvas as content
*/
class CanvasViewContent : public AbstractContent, public SingleResourceLoaner
{
    Q_OBJECT
    public:
        CanvasViewContent(bool spontaneous, QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~CanvasViewContent();

        bool loadFromFile(const QString & fwFilePath, bool keepRatio = false, bool setName = false);

        // ::AbstractContent
        QString contentName() const { return tr("Canvas View"); }
        bool fromXml(QDomElement & contentElement, const QDir & baseDir);
        void toXml(QDomElement & contentElement, const QDir & baseDir) const;
        void drawContent(QPainter * painter, const QRect & targetRect, Qt::AspectRatioMode ratio);
        int contentHeightForWidth(int width) const;

        // ::SingleResourceProvider
        QVariant takeResource();
        void returnResource(const QVariant &);

    private:
        Canvas * m_canvas;
        QSize m_canvasCachedSize;
        bool m_canvasTaken;

    private Q_SLOTS:
        void slotRepaintCanvas(const QList<QRectF> & exposed);
};

#endif
