/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
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
class QAbstractButton;
class QListWidgetItem;
class StyledButtonItem;
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
        void addTab(QWidget * widget, const QString & label, bool front = false, bool setCurrent = false);
        void showOkButton(bool show);

        // ::QGraphicsProxyWidget
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        void resizeEvent(QGraphicsSceneResizeEvent * event);

    protected Q_SLOTS:
        virtual void slotOkClicked() {};
        void slotRequestClose();

    private:
        void populateFrameList();
        void layoutButtons();
        AbstractContent *       m_content;
        Ui::AbstractConfig *    m_commonUi;
        StyledButtonItem *      m_closeButton;
        StyledButtonItem *      m_okButton;
        Frame *                 m_frame;

    private Q_SLOTS:
        void on_newFrame_clicked();
        void on_applyLooks_clicked();
        void on_listWidget_itemSelectionChanged();
        void on_reflection_toggled(bool checked);
};

#endif
