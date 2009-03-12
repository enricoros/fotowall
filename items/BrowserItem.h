/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Inspired from Plasma Web Applet <www.kde.org> and Qt LGPL sources     *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __BrowserItem_h__
#define __BrowserItem_h__

#include <QGraphicsWidget>
#include <QUrl>
class WebPage;

class BrowserItem : public QGraphicsWidget
{
    Q_OBJECT
    public:
        BrowserItem(QGraphicsItem * parent = 0);

        // browser actions
        void write(const QString & html, const QUrl & baseUrl = QUrl());
        void browse(const QString & url);
        void historyBack();
        void historyForward();

        // read only (NOTE: this is raw, set this ASAP and forget ;-)
        bool readOnly() const;
        void setReadOnly(bool on);

    Q_SIGNALS:
        void linkClicked(const QUrl & url);

    protected:
        // ::QGraphicsWidget
        void resizeEvent(QGraphicsSceneResizeEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

    private:
        WebPage * m_webPage;
        bool m_readOnly;

    private Q_SLOTS:
        void slotScrollRequested(int dx, int dy, const QRect & scrollViewRect);
        void slotUpdateRequested(const QRect & dirtyRect);

    // ::QGaphicsItem -> to QtWebkit
    private:
        // focus
        void focusInEvent(QFocusEvent * event);
        void focusOutEvent(QFocusEvent * event);

        // keyboard
        void keyPressEvent(QKeyEvent * event);
        void keyReleaseEvent(QKeyEvent * event);
        void inputMethodEvent(QInputMethodEvent * event);
        QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

        // mouse
        void hoverMoveEvent(QGraphicsSceneHoverEvent * event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);
        void wheelEvent(QGraphicsSceneWheelEvent * event);

        // drag & drop
        void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
        void dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
        void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
        void dropEvent(QGraphicsSceneDragDropEvent * event);

        // glue and tricks
        bool sceneEvent(QEvent * event);
};

#endif
