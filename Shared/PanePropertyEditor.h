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

#ifndef __PanePropertyEditor_h__
#define __PanePropertyEditor_h__

#include "PropertyEditors.h"
#include <QWidget>
#include <QPointF>
#include <QRectF>

/// Pane Widget, a cartesian point selector
class PaneWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPointF value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QRectF range READ range WRITE setRange NOTIFY rangeChanged)
    public:
        PaneWidget(QWidget * parent = 0);

        QPointF value() const;
        QPointF endValue() const;
        void setValue(const QPointF &);

        QRectF range() const;
        void setRange(const QRectF &);

    Q_SIGNALS:
        void valueChanged(const QPointF &);
        void rangeChanged(const QRectF &);
        void pressed();
        void released();

    protected:
        void enterEvent(QEvent *);
        void leaveEvent(QEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseDoubleClickEvent(QMouseEvent *);
        void paintEvent(QPaintEvent * event);

    private:
        void pressing(const QPointF & pos);
        QPointF screenMap(const QPointF & value) const;
        QRectF m_range;
        QPointF m_value;
        QPointF m_endValue;
        bool m_hovered;
        bool m_pressed;
};

/// Pane Property Editor, links a property to a PaneWidget
class PE_PaneWidget : public PE_TypeControl<PaneWidget>
{
    Q_OBJECT
    public:
        PE_PaneWidget(PaneWidget * pane, QObject * target, const char * propertyName, QObject * parent = 0);

    private Q_SLOTS:
        void slotPaneValueChanged(const QPointF &);
        void slotPropertyChanged();
};

#endif
