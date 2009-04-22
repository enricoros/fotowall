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

#ifndef __TextContent_h__
#define __TextContent_h__

#include "AbstractContent.h"
class QTextDocument;

/// \brief TODO
class TextContent : public AbstractContent
{
    Q_OBJECT
    public:
        TextContent(QGraphicsScene * scene, QGraphicsItem * parent = 0);
        ~TextContent();

        QString toHtml() const;
        void setHtml(const QString & htmlCode);

        // ::AbstractContent
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        QPixmap renderAsBackground(const QSize & size, bool keepAspect) const;
        int contentHeightForWidth(int width) const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        QTextDocument * m_text;
        QSizeF m_textSize;

    private Q_SLOTS:
        void slotTextResized(const QSizeF & size);
};

#endif
