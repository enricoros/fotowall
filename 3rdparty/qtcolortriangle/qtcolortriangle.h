/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      file             : qtcolortriangle.h                               *
 *      license          : LGPL v2.1 and GPL v3                            *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

#ifndef QTCOLORTRIANGLE_H
#define QTCOLORTRIANGLE_H
#include <QtGui/QImage>
#include <QtWidgets/QWidget>

class QPointF;
struct Vertex;

class QtColorTriangle : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)

public:
    QtColorTriangle(QWidget *parent = 0);
    ~QtColorTriangle();

    QSize sizeHint() const;
    int heightForWidth(int w) const;

    void polish();
    QColor color() const;

Q_SIGNALS:
    void colorChanged(const QColor &col);

public Q_SLOTS:
    void animateSetColor(const QColor &col);
    void setColor(const QColor &col);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *);
    void drawTrigon(QImage *p, const QPointF &a, const QPointF &b,
		    const QPointF &c, const QColor &color);

private:
    double radiusAt(const QPointF &pos, const QRect &rect) const;
    double angleAt(const QPointF &pos, const QRect &rect) const;
    QPointF movePointToTriangle(double x, double y, const Vertex &a,
				    const Vertex &b, const Vertex &c) const;

    QPointF pointFromColor(const QColor &col) const;
    QColor colorFromPoint(const QPointF &p) const;

    void genBackground();

    QImage bg;
    double a, b, c;
    QPointF pa, pb, pc, pd;

    QColor curColor;
    int curHue;

    bool mustGenerateBackground;
    int penWidth;
    int ellipseSize;

    int outerRadius;
    QPointF selectorPos;

    enum SelectionMode {
	Idle,
	SelectingHue,
	SelectingSatValue
    } selMode;
};


#include <QtWidgets/QColorDialog>

class QColorTriangleDialog : public QDialog
{
    public:
        static QRgb customColor(int index);
        static int customCount();
        static QColor getColor(const QColor & initial, QWidget * parent, const QString & title, QColorDialog::ColorDialogOptions options = 0);
        static QColor getColor(const QColor & initial = Qt::white, QWidget * parent = 0);
        static void setCustomColor(int index, QRgb color);
        static void setStandardColor(int index, QRgb color);
};

#endif
