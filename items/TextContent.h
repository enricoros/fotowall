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
        void save(QDataStream & data) const;
        bool restore(QDataStream & data);
        QPixmap renderAsBackground(const QSize & size) const;

        // ::QGraphicsItem
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    protected:
/*        bool sceneEvent(QEvent *event);
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);
        void focusInEvent(QFocusEvent *event);
        void focusOutEvent(QFocusEvent *event);
        void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
        void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent *event);
        void dropEvent(QGraphicsSceneDragDropEvent *event);
        void inputMethodEvent(QInputMethodEvent *event);
        void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

        QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
*/
    private:
        QTextDocument * m_text;

    private Q_SLOTS:
        void slotResize(const QSizeF & size);
};

#endif
