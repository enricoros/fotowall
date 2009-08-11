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

#ifndef __GroupBoxWidget_h__
#define __GroupBoxWidget_h__

#include <QWidget>
#include <QFont>

class GroupBoxWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(int titleSize READ titleSize WRITE setTitleSize)
    Q_PROPERTY(qreal shading READ shading WRITE setShading)
    public:
        GroupBoxWidget(QWidget * parent = 0);

        QString title() const;
        void setTitle(const QString & title);

        int titleSize() const;
        void setTitleSize(int titleSize);

        qreal shading() const;
        void setShading(qreal value);

    protected:
        void enterEvent(QEvent *);
        void leaveEvent(QEvent *);
        void paintEvent(QPaintEvent * event);

    private:
        QString m_titleText;
        QFont m_titleFont;
        qreal m_shading;
};

#endif
