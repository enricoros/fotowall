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

#ifndef __DeskViewContent_h__
#define __DeskViewContent_h__

#include "AbstractContent.h"
#include <QPixmap>
class Desk;

/**
    \brief Displays a Desk
*/
class DeskViewContent : public AbstractContent
{
    Q_OBJECT
    public:
        DeskViewContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
//        ~DeskViewContent();

        bool load(const QString & filePath, bool keepRatio = false, bool setName = false);
        //Desk * takeDesk();
        //void setDesk(Desk * desk);

        // ::AbstractContent
        QString contentName() const { return tr("DeskXXX"); }
        QWidget * createPropertyWidget();
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        QPixmap renderContent(const QSize & size, Qt::AspectRatioMode ratio) const;

//        int contentHeightForWidth(int width) const;
        bool contentOpaque() const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        Desk * m_innerDesk;
};

#endif
