/******************************************************************************
     Copyright (C) 2008  TANGUY Arnaud arn.tanguy@gmail.com
*                                                                             *
* This program is free software; you can redistribute it and/or modify        *
* it under the terms of the GNU General Public License as published by        *
* the Free Software Foundation; either version 2 of the License, or           *
* (at your option) any later version.                                         *
*                                                                             *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
* GNU General Public License for more details.                                *
*                                                                             *
* You should have received a copy of the GNU General Public License along     *
* with this program; if not, write to the Free Software Foundation, Inc.,     *
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                 *
 ******************************************************************************/

#ifndef __CroppingWidget__
#define __CroppingWidget__

#include <QWidget>
class QRubberBand;
class CPixmap;

/*
 * This widget provides a way to select a region of a photo
 * */
class CroppingWidget : public QWidget {
    private:
        QRubberBand *m_rubberBand;
        CPixmap *m_photo;
        QPixmap m_previewPixmap;
        QPoint m_origin;
        float m_previewRatio;
    public:
        CroppingWidget(QWidget *parent=0);
        void setPixmap(CPixmap *pix);
        QRect getCroppingRect() const;
    protected:
         void mousePressEvent(QMouseEvent *event);
         void mouseMoveEvent(QMouseEvent *event);
         void mouseReleaseEvent(QMouseEvent *event);
         void paintEvent(QPaintEvent *event);
};


#endif
