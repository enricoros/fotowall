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

class GroupBoxWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(int titleSize READ titleSize WRITE setTitleSize)
    Q_PROPERTY(int fixedWidth READ minimumWidth WRITE setFixedWidth)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(int borderFlags READ borderFlags WRITE setBorderFlags)
    public:
        GroupBoxWidget(QWidget * parent = 0);

        QString title() const;
        void setTitle(const QString & title);

        int titleSize() const;
        void setTitleSize(int titleSize);

        bool isCheckable() const;
        void setCheckable(bool checkable);

        bool isChecked() const;
        void setChecked(bool checked);

        int borderFlags() const;
        void setBorderFlags(int flags);

        // an elegant alternative to show and hide
        void collapse();
        void expand();

    Q_SIGNALS:
        void toggled(bool on);

    protected:
        void enterEvent(QEvent *);
        void leaveEvent(QEvent *);
        void mousePressEvent(QMouseEvent *);
        void paintEvent(QPaintEvent * event);

    private:
        // for internal animation only
        Q_PROPERTY(qreal cAnim READ checkValue WRITE setCheckValue)
        Q_PROPERTY(qreal hAnim READ hoverValue WRITE setHoverValue)
        qreal checkValue() const;
        void setCheckValue(qreal value);
        qreal hoverValue() const;
        void setHoverValue(qreal value);

        int calcMinWidth() const;
        void updateDesign();
        QTimer * m_redesignTimer;
        QString m_titleText;
        QFont m_titleFont;
        bool m_collapsed;
        bool m_checkable;
        bool m_checked;
        int m_borderFlags;
        qreal m_checkValue;
        qreal m_hoverValue;

   private Q_SLOTS:
        void slotAnimateDesign();
        void slotFinalizeDesign();
};


#endif
