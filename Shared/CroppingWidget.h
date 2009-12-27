/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CroppingWidget__
#define __CroppingWidget__

#include <QGraphicsView>
#include <QGraphicsWidget>
class QGraphicsScene;
class MaskItem;
class RubberBandItem;

/**
 \brief This widget provides a way to select a region of a photo
*/
class CroppingWidget : public QGraphicsView {
    public:
        CroppingWidget(QWidget *parent=0);
        void setPixmap(QPixmap *pix);
        QRect getCroppingRect() const;

    protected:
        // ::QGraphicsView
        void drawBackground(QPainter *painter, const QRectF &rect);
        void resizeEvent(QResizeEvent *event);

    private:
        QGraphicsScene * m_scene;
        MaskItem * m_maskItem;
        RubberBandItem * m_rubberBand;
        QPixmap m_previewPixmap;
        QPoint m_origin;
        float m_previewScale;
};


// The following are here for MOC puropose only

class MaskItem : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal strength READ strength WRITE setStrength)

public slots:
    void resizeToScene();
    void setHole(const QRect & rect);

public:
    MaskItem();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget *widget = 0);

private:
    qreal strength() const;
    void setStrength(qreal strength);
    QRect m_hole;
    qreal m_strength;
};


class RubberBandItem : public QGraphicsWidget
{
    Q_OBJECT
signals:
    void geometryChanged(const QRect & rect);

public:
    RubberBandItem();
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent * event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

private:
    QList<QRect> m_corners;
    QPoint m_displace;
    int m_handle;
};

#endif
