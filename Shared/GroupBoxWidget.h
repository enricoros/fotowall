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

#ifndef __GroupBoxWidget_h__
#define __GroupBoxWidget_h__

#include <QWidget>
#include <QFont>
class QTimer;

/**
  This file is very different from the Desktop to the Mobile version:
   - on desktop it's an expanding collapsible fixed-height horizontal container
   - on mobile it's like a dockable panel that is raised when clicking on the top label
*/

class GroupBoxWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(int titleSize READ titleSize WRITE setTitleSize)
    Q_PROPERTY(int fixedWidth READ minimumWidth WRITE setFixedWidth)
#if !defined(MOBILE_UI)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(int borderFlags READ borderFlags WRITE setBorderFlags)
#else
    Q_PROPERTY(QPoint basePos READ basePos WRITE setBasePos);
#endif
    public:
        GroupBoxWidget(QWidget * parent = 0);

        QString title() const;
        void setTitle(const QString & title);

        int titleSize() const;
        void setTitleSize(int titleSize);

        int calcMinWidth() const;

#if !defined(MOBILE_UI)
        bool isCheckable() const;
        void setCheckable(bool checkable);

        bool isChecked() const;
        void setChecked(bool checked);

        int borderFlags() const;
        void setBorderFlags(int flags);

        // an elegant alternative to show and hide
        void collapse();
        void expand();
#else
        void setSmartPanel(bool);

        QPoint basePos() const;
        void setBasePos(const QPoint &);

        int labelWidth() const;

        void disappear();

        void smartFall();
        void smartRaise();
#endif

    Q_SIGNALS:
#if !defined(MOBILE_UI)
        void toggled(bool on);
#else
        void labelSizeChanged();
        void panelRaising();
        void panelLowering();
#endif

    protected:
        void mousePressEvent(QMouseEvent *);
        void paintEvent(QPaintEvent * event);
#if !defined(MOBILE_UI)
        void enterEvent(QEvent *);
        void leaveEvent(QEvent *);
#endif

    private:
        // for internal animation only
#if !defined(MOBILE_UI)
        Q_PROPERTY(qreal cAnim READ checkValue WRITE setCheckValue)
        qreal checkValue() const;
        void setCheckValue(qreal value);
        Q_PROPERTY(qreal hAnim READ hoverValue WRITE setHoverValue)
        qreal hoverValue() const;
        void setHoverValue(qreal value);
#else
        Q_PROPERTY(qreal panelState READ panelState WRITE setPanelState);
        qreal panelState() const;
        void setPanelState(qreal);
#endif
        void updateDesign();
        QTimer * m_redesignTimer;
        QString m_titleText;
        QFont m_titleFont;
#if !defined(MOBILE_UI)
        bool m_collapsed;
        bool m_checkable;
        bool m_checked;
        int m_borderFlags;
        qreal m_checkValue;
        qreal m_hoverValue;
#else
        int verticalOffset() const;
        bool m_smartPanel;
        QRect m_labelRect;
        QPoint m_basePos;
        qreal m_panelState;
#endif

   private Q_SLOTS:
        void slotAnimateDesign();
        void slotFinalizeDesign();
};

#endif
