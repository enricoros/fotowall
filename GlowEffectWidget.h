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
#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QPixmap>
#include <QImage>

class GlowEffectWidget : public QWidget
{
    Q_OBJECT
public:
    GlowEffectWidget(QWidget *parent=0);
    void setPreviewImage(const QImage &preview);
    QImage glow(const QImage &image, int radius) const;
    void setGlowRadius(int radius);
    int glowRadius() const;
protected:
    void paintEvent(QPaintEvent *e);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);

    void drawRadius(QPainter *p);

    void generateLens(const QRectF &bounds);
private:
    QImage m_image;

    int  m_radius;

    QPixmap m_tile;
    bool m_mouseIn;
    bool m_mouseDown;
    QImage m_lens;

    QPointF m_pos;
};


#endif
