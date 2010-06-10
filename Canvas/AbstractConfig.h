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

#ifndef __AbstractConfig_h__
#define __AbstractConfig_h__

#include <QBasicTimer>
#include <QGraphicsProxyWidget>
#include <QWidget>
class AbstractContent;
class Frame;
class QAbstractButton;
class QListWidgetItem;
class StyledButtonItem;
namespace Ui { class AbstractConfig; }

#if !defined(MOBILE_UI)
class AbstractConfig : public QGraphicsProxyWidget {
#else
class AbstractConfig : public QWidget {
#endif
    Q_OBJECT
    public:
#if !defined(MOBILE_UI)
        AbstractConfig(AbstractContent * content, QGraphicsItem * parent = 0);
#else
        AbstractConfig(AbstractContent * content, QWidget * parent = 0);
#endif
        virtual ~AbstractConfig();

        void dispose();

        // the related content
        AbstractContent * content() const;

        // manage property box
        void keepInBoundaries(const QRectF & rect);

    Q_SIGNALS:
        void applyLook(quint32 frameClass, bool mirrored, bool allContents);
        void createNewFrame(QStringList);
        void requestClose();

    protected:
        // used by subclasses
        void addTab(QWidget * widget, const QString & label, bool front = false, bool setCurrent = false);
        void showOkButton(bool show);

        // ::QGraphicsProxyWidget
#if !defined(MOBILE_UI)
        void mousePressEvent(QGraphicsSceneMouseEvent * event);
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
        void resizeEvent(QGraphicsSceneResizeEvent * event);
#endif

    protected Q_SLOTS:
        virtual void slotOkClicked() {}

    private:
        void populateFrameList();
        void layoutButtons();
        QPixmap                 m_backPixmap;
        AbstractContent *       m_content;
        Ui::AbstractConfig *    m_commonUi;
#if !defined(MOBILE_UI)
        StyledButtonItem *      m_closeButton;
        StyledButtonItem *      m_okButton;
#endif
        Frame *                 m_frame;

    private Q_SLOTS:
        void slotAddFrame();
        void slotLookApplyAll();
        void slotFrameSelectionChanged();
        void slotReflectionToggled(bool checked);
};

#endif
