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

#include "bezier.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPainterPathStroker>
#include <QPainterPath>
#include <QBrush>
#include <QPen>
#include <QImage>
#include <QPixmap>
#include <QLineEdit>
#include <QTimer>

#include <QtDebug>

#include <math.h>

static const int pointSize = 5;

//derivative of the equation
qreal slopeAt(qreal t, qreal a, qreal b, qreal c, qreal d)
{
    return 3*t*t*(d-3*c+3*b-a)+6*t*(c-2*b+a)+3*(b-a);
}

Bezier::Bezier(QWidget *parent)
    : QWidget(parent)
{
    m_tile = QPixmap(100, 100);
    m_tile.fill(Qt::white);
    QPainter pt(&m_tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, 50, 50, color);
    pt.fillRect(50, 50, 50, 50, color);
    pt.end();

//#if 0
    one = QPointF(50, 50);
    two = QPointF(150, 150);
    three = QPointF(100, 250);
    four = QPointF(300, 50);
//#else
    //one   = QPointF(157, 211);
    //two   = QPointF(150, 150);
    //three = QPointF(329,  50);
    //four  = QPointF(335, 126);

    //one   = QPointF(153, 199);
    //two   = QPointF(153, 100);
    //three = QPointF(300, 100);
    //four  = QPointF(300, 199);

//     one   = QPointF(153, 199);
//     two   = QPointF(147, 61);
//     three = QPointF(414, 18);
//     four  = QPointF(355, 201);

    //one   = QPointF(153, 199);
    //two   = QPointF(59, 53);
    //three = QPointF(597, 218);
    //four  = QPointF(355, 201);


//#endif

    setMouseTracking(true);
    moving = 0;
    onThePath = false;
    currentT = 0.5;

    m_fontBox = new QFontComboBox(this);
    connect(m_fontBox, SIGNAL(currentFontChanged(const QFont&)),
            SLOT(changeFont(const QFont &)));

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setText(tr("Change this text"));
    connect(m_lineEdit, SIGNAL(textChanged(const QString&)),
            SLOT(update()));

    m_fontSize = new QSpinBox(this);
    m_fontSize->setValue(12);
    m_fontSize->setMinimum(6);
    m_fontSize->setMaximum(128);
    connect(m_fontSize, SIGNAL(valueChanged(int)),
            SLOT(update()));
}

/* FotoWall */
QPainterPath Bezier::path() const
{
    return m_path;
}
QFont Bezier::font() const
{
    return m_fontBox->currentFont();
}
void Bezier::setFont(const QFont &font)
{
    m_fontBox->setCurrentFont(font);
}
int Bezier::fontSize() const
{
    return m_fontSize->value();
}
void Bezier::setFontSize(int size)
{
    m_fontSize->setValue(size);
}
QString Bezier::text() const
{
    return m_lineEdit->text();
}
void Bezier::setText(const QString &text)
{
    m_lineEdit->setText(text);
}

QList<QPointF> Bezier::controlPoints() const
{
    QList<QPointF> controlPts;
    controlPts << one << two << three << four;
    return controlPts;
}
void Bezier::setControlPoints(const QList<QPointF> &controlPts)
{
    if (controlPts.length() == 4) {
        one = controlPts[0];
        two = controlPts[1];
        three = controlPts[2];
        four = controlPts[3];
    }
}
/*  /FotoWall */

void Bezier::drawBackground(QPainter *painter, const QRectF &fullRect)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawTiledPixmap(fullRect, m_tile);
    painter->restore();
}

void Bezier::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    drawBackground(&p, rect());
    m_path = QPainterPath(one);

    m_path.cubicTo(two, three, four);
    p.strokePath(m_path, QPen(Qt::red, 2));

    drawLine(&p, one, two);
    drawLine(&p, four, three);

    drawPoint(&p, four);


    drawPoint(&p, one);
    drawPoint(&p, two);
    drawPoint(&p, three);

    QFont font = m_fontBox->currentFont();
    font.setPointSize(m_fontSize->value());
    p.setFont(font);
    QString str = m_lineEdit->text();
    QFontMetricsF metrics(p.font());
    //qreal curLen = 0;
    qreal curLen = 20;
    //qDebug()<<one<<two<<three<<four;
    for (int i = 0; i < str.length(); ++i) {
        qreal t = m_path.percentAtLength(curLen);
        QPointF pt = m_path.pointAtPercent(t);
        //qreal angle = m_path.angleAtPercent(t);
        qreal angle = -m_path.angleAtPercent(t);

        QString txt;
        txt.append(str[i]);
        p.save();
        p.translate(pt);
        //qDebug()<<"txt = "<<txt<<", angle = "<<angle<<curLen<<t;
        p.rotate(angle);
        p.drawText(0, 0, txt);
        p.restore();

        qreal incremenet = metrics.width(txt);
        curLen += incremenet;
    }
    drawFrames(&p);
    //QTimer::singleShot(1000/30, this, SLOT(update()));
}

void Bezier::drawFrames(QPainter *p)
{
    QFont font("ComicSans", 8);
    const QSize  bsize(160, 100);
    const QRect boxRect(10, rect().height()-bsize.height()-10,
                         bsize.width(), bsize.height());

    p->save();
    p->setRenderHint(QPainter::Antialiasing);
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setPen(QPen(Qt::black));
    p->setBrush(QBrush(QColor(193, 193, 193, 127)));
    p->drawRoundRect(boxRect, 25*boxRect.height()/boxRect.width(), 25);

    font.setPointSize(10);
    font.setUnderline(true);
    p->setFont(font);
    p->drawText(boxRect.x()+10, boxRect.y()+15, "Info:");

    p->restore();
}

void Bezier::drawPoint(QPainter *painter, const QPointF &pos)
{
    painter->save();
    painter->setPen(QColor(50, 100, 120, 200));
    painter->setBrush(QColor(200, 200, 210, 120));
    painter->drawEllipse(QRectF(pos.x() - pointSize,
                                pos.y() - pointSize,
                                pointSize*2, pointSize*2));
    painter->restore();
}

void Bezier::drawLine(QPainter *painter, const QPointF &start,
                      const QPointF &end)
{
    painter->save();
    QPen pen(QColor(255, 0, 255, 127), 1, Qt::DashLine);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(start, end);
    painter->restore();
}

void Bezier::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (markContains(one, e->pos())) {
            //qDebug()<<"one";
            moving = &one;
        } else if (markContains(two, e->pos())) {
            //qDebug()<<"two";
            moving = &two;
        } else if (markContains(three, e->pos())) {
            //qDebug()<<"three";
            moving = &three;
        } else if (markContains(four, e->pos())) {
            //qDebug()<<"four";
            moving = &four;
        } else {
            QPainterPathStroker stroker;
            stroker.setWidth(2);
            QPainterPath path(one);
            path.cubicTo(two, three, four);

            QPainterPath stroked = stroker.createStroke(path);
            if (stroked.contains(e->pos())) {
                //qDebug()<<"on the path";
                onThePath = true;
            }
        }

        if (moving || onThePath)
            mouseStart = e->pos();
    }
}

void Bezier::mouseMoveEvent(QMouseEvent *e)
{
    if (moving) {
        QPointF diff = e->pos() - mouseStart;
        *moving += diff;
        QVector<QPointF> bezier(4);
        bezier[0] = one;
        bezier[1] = two;
        bezier[2] = three;
        bezier[3] = four;
        //currentT = tAtPointOnCurve(tPoint, bezier);
        update();
        mouseStart = e->pos();
    } else if (onThePath) {
        QPointF diff = e->pos() - mouseStart;
        one   += diff;
        two   += diff;
        three += diff;
        four  += diff;
        QVector<QPointF> bezier(4);
        bezier[0] = one;
        bezier[1] = two;
        bezier[2] = three;
        bezier[3] = four;
        //currentT = tAtPointOnCurve(currentT, bezier);
        update();
        mouseStart = e->pos();
    }
}

void Bezier::mouseReleaseEvent(QMouseEvent *)
{
    //qDebug()<<"releasing";
    moving = 0;
    onThePath = false;
}

bool Bezier::markContains(const QPointF &pos, const QPointF &coord) const
{
    QRectF rect(pos.x() - pointSize,
                pos.y() - pointSize,
                pointSize*2, pointSize*2);
    QPainterPath path;
    path.addEllipse(rect);
    return path.contains(coord);
}

void Bezier::resizeEvent(QResizeEvent *e)
{
    QSize s = e->size();

    m_fontBox->setGeometry(20, rect().height() - 85, 140, 20);
    m_lineEdit->setGeometry(20, rect().height() - 60, 140, 20);
    m_fontSize->setGeometry(20, rect().height() - 35, 140, 20);

}

void Bezier::changeFont(const QFont &font)
{
    Q_UNUSED(font);
    update();
}
