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

#ifndef __AbstractProperties_h__
#define __AbstractProperties_h__

#include <QGraphicsProxyWidget>
#include <QBasicTimer>
class AbstractContent;
class Frame;
class PixmapButton;
class QAbstractButton;
class QListWidgetItem;
namespace Ui { class AbstractProperties; }


class AbstractProperties : public QGraphicsProxyWidget {
    Q_OBJECT
    public:
        AbstractProperties(AbstractContent * content, QGraphicsItem * parent = 0);
        virtual ~AbstractProperties();

        // the current content
        AbstractContent * content() const;

        // manage property box
        void keepInBoundaries(const QRect & rect);
        void animateClose();

    Q_SIGNALS:
        void closing();
        void closed();
        void applyLooks(quint32 frameClass, bool mirrored);
        void applyLookToSelection(quint32 frameClass, bool mirrored);
        void reflexionToogled(bool);

    protected:
        // used by subclasses
        void addTab(QWidget * widget, const QString & label, bool front = false, bool select = false);

        // ::QGraphicsProxyWidget
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        void resizeEvent(QGraphicsSceneResizeEvent * event);
        void timerEvent(QTimerEvent *);

    private:
        AbstractContent *           m_content;
        Ui::AbstractProperties *    m_commonUi;
        PixmapButton *              m_closeButton;
        Frame *                     m_frame;
        int                         m_aniStep;
        bool                        m_aniDirection;
        QBasicTimer                 m_aniTimer;

    private Q_SLOTS:
        void on_applyLooks_clicked();
        void on_listWidget_itemSelectionChanged();
        void on_reflection_toggled(bool checked);
};

#endif
