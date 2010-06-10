/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009-2010 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __SmartPanel_h__
#define __SmartPanel_h__

#include <QWidget>
#include <QPoint>

class SmartPanel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPoint basePos READ basePos WRITE setBasePos);
    Q_PROPERTY(qreal panelState READ panelState WRITE setPanelState);
public:
    explicit SmartPanel(const QString &label, QWidget *embeddedWidget, QWidget *parent = 0);
    ~SmartPanel();

    QPoint basePos() const { return m_basePos; }
    void setBasePos(const QPoint &);

    int labelWidth() const { return m_labelRect.width(); }

    QWidget *embeddedWidget() const { return m_embeddedWidget; }

signals:
    void panelLowering();
    void panelRaising();

protected:
    bool eventFilter(QObject *, QEvent *);
    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);

private:
    void setPanelState(qreal);
    qreal panelState() const { return m_panelState; }

    int verticalOffset() const;
    void updateLayout();
    // setup
    QString m_text;
    QRect m_labelRect;
    QPoint m_basePos;
    QWidget *m_embeddedWidget;
    // state
    qreal m_panelState;
    QSize m_minSize;
};

#endif // __SmartPanel_h__
