/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CroppingWidget__
#define __CroppingWidget__

#include <QWidget>
class QRubberBand;

/*
 * This widget provides a way to select a region of a photo
 * */
class CroppingWidget : public QWidget {
    private:
        QRubberBand *m_rubberBand;
        QPixmap m_previewPixmap;
        QPoint m_origin;
        float m_previewRatio;
    public:
        CroppingWidget(QWidget *parent=0);
        void setPixmap(QPixmap *pix);
        QRect getCroppingRect() const;
    protected:
         void mousePressEvent(QMouseEvent *event);
         void mouseMoveEvent(QMouseEvent *event);
         void mouseReleaseEvent(QMouseEvent *event);
         void paintEvent(QPaintEvent *event);
};


#endif
