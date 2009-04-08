/*****************************************************************************
* Copyright (C) 2008 TANGUY Arnaud <arn.tanguy@gmail.com>                    *
*                                                                            *
* This program is free software; you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation; either version 2 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License along    * 
* with this program; if not, write to the Free Software Foundation, Inc.,    *
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                *
******************************************************************************/

#ifndef ARNAUD_GLOWEFFECT_H
#define ARNAUD_GLOWEFFECT_H

#include <QGLWidget>
#include <QPaintEvent>
#include <QPixmap>
#include <QImage>
#include "CPixmap.h"
#include <QDialog>
#include <QSpinBox>
#include "ui_GlowEffectDialog.h"

class GlowEffect : public QDialog
{
    Q_OBJECT
public:
    GlowEffect(CPixmap *image, QWidget *parent=0);
    QImage glow();
    QSize sizeHint() const;
protected:
    void generateLens(const QRectF &bounds);

public slots:
    void slotRadiusValueChanged(int);
    void slotPreview();
    void slotRender();

private:
    int  m_radius;
    CPixmap *m_image;
    QSpinBox *m_radiusSpinBox;
    Ui::GlowEffectDialog ui;

    QImage m_blurred;
    QPixmap m_tile;
};

#endif 

