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
#include <QPixmap>
class Canvas;

/**
    \brief Use another Canvas as content
*/
class CanvasViewContent : public AbstractContent
{
    Q_OBJECT
    public:
        CanvasViewContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
//        ~CanvasViewContent();

        bool load(const QString & filePath, bool keepRatio = false, bool setName = false);
        //Canvas * takeCanvas();
        //void setCanvas(Canvas * canvas);

        // ::AbstractContent
        QString contentName() const { return tr("CanvasXXX"); }
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
        Canvas * m_canvas;

    private Q_SLOTS:
        void slotRepaintCanvas(const QList<QRectF> & exposed);
};

#endif
