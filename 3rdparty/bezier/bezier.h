/****************************************************************************
**
** Copyright (C) 2007-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Graphics Dojo project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef BEZIER_H
#define BEZIER_H

#include <QWidget>
#include <QPointF>
#include <QPixmap>
#include <QFontComboBox>
#include <QSpinBox>
#include <QMovie>
#include <QList>

class QCheckBox;
class QPaintEvent;
class QPainter;
class QMouseEvent;
class QRectF;

class Bezier : public QWidget
{
    Q_OBJECT
public:
    Bezier(QWidget *parent=0);

    //FotoWall
    QPainterPath path() const;
    QFont font() const;
    void setFont(const QFont &);
    QString text() const;
    void setText(const QString &);
    QList<QPointF> controlPoints() const;
    void setControlPoints(const QList<QPointF> &);
    bool enabled() const;
    void setEnabled(bool);
    void focusLineEdit();

public slots:
    void changeFont(const QFont &font);

protected slots:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawPoint(QPainter *painter, const QPointF &pt);
    void drawLine(QPainter *painter, const QPointF &start,
                  const QPointF &end);
    bool markContains(const QPointF &pt, const QPointF &coord) const;
    void drawFrames(QPainter *p);

private:
    QPointF one, two, three, four;
    QPixmap m_tile;
    QPainterPath m_path;

    QPointF mouseStart;
    QPointF *moving;
    bool     onThePath;

    qreal   currentT;

    QCheckBox     *m_enableBox;
    QFontComboBox *m_fontBox;
    QSpinBox      *m_fontSize;
    QLineEdit     *m_lineEdit;
};

#endif
