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
        QString toPlainText() const;
        void setHtml(const QString & htmlCode);
        QFont defaultFont() const;

        // bezier shape controls
        void setShapeEnabled(bool enabled);
        void setShapePath(const QPainterPath & path);
        void setShapeControlPoints(const QList<QPointF> & points);
        bool shapeEnabled() const;
        QPainterPath shapePath() const;
        QList<QPointF> shapeControlPoints() const;

        // ::AbstractContent
        bool fromXml(QDomElement & parentElement);
        void toXml(QDomElement & parentElement) const;
        QPixmap renderAsBackground(const QSize & size, bool keepAspect) const;
        int contentHeightForWidth(int width) const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        void updateTextConstraints();
        void updateCache();

        // text document, layouting & rendering
        QTextDocument * m_text;
        QList<QRect> m_blockRects;
        QRect m_textRect;
        int m_textMargin;

        // shape related stuff
        bool m_shapeEnabled;
        QList<QPointF> m_shapeControlPoints;
        QPainterPath m_shapePath;
        QRect m_shapeRect;
};

#endif
