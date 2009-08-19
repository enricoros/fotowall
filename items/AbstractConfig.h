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

#ifndef __AbstractConfig_h__
#define __AbstractConfig_h__

#include <QGraphicsProxyWidget>
#include <QBasicTimer>
class AbstractContent;
class Frame;
class PixmapButton;
class QAbstractButton;
class QListWidgetItem;
namespace Ui { class AbstractConfig; }


class AbstractConfig : public QGraphicsProxyWidget {
    Q_OBJECT
    public:
        AbstractConfig(AbstractContent * content, QGraphicsItem * parent = 0);
        virtual ~AbstractConfig();

        void dispose();

        // the related content
        AbstractContent * content() const;

        // manage property box
        void keepInBoundaries(const QRect & rect);

    Q_SIGNALS:
        void applyLook(quint32 frameClass, bool mirrored, bool allContents);
        void createNewFrame(QStringList);

    protected:
        // used by subclasses
        virtual void closing() {};
        void addTab(QWidget * widget, const QString & label, bool front = false, bool setCurrent = false);

        // ::QGraphicsProxyWidget
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        void resizeEvent(QGraphicsSceneResizeEvent * event);

    private:
        AbstractContent *       m_content;
        Ui::AbstractConfig *    m_commonUi;
        PixmapButton *          m_closeButton;
        Frame *                 m_frame;

        void populateFrameList();

    private Q_SLOTS:
        void on_newFrame_clicked();
        void on_applyLooks_clicked();
        void on_listWidget_itemSelectionChanged();
        void on_reflection_toggled(bool checked);
};

#endif
