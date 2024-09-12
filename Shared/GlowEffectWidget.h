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
 *      This code was inspired from an example on the Graphics Dojo page   *
 *      of the Qt Software Labs by Zack Rusin. The code was released       *
 *      under the GNU GPL version 2.0.                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef __GlowEffectWidget_h__
#define __GlowEffectWidget_h__

#include <QImage>
#include <QPixmap>
#include <QWidget>
class QPaintEvent;

class GlowEffectWidget : public QWidget
{
  Q_OBJECT
public:
  GlowEffectWidget(QWidget * parent = 0);
  void setPreviewImage(const QImage & preview);
  void setGlowRadius(int radius);
  int glowRadius() const;

  static QImage glown(const QImage & image, int radius);
  static QImage dropShadow(const QImage & image,
                           const QColor & shadowColor,
                           int radius,
                           int xOffset = 0,
                           int yOffset = 0);

protected:
  // ::QWidget
  void mousePressEvent(QMouseEvent * e);
  void mouseReleaseEvent(QMouseEvent * e);
  void paintEvent(QPaintEvent * e);
  void wheelEvent(QWheelEvent * e);

private:
  void drawRadiusBox(QPainter * p);
  QImage m_image;
  int m_radius;
  QPixmap m_tile;
  bool m_mousePressed;
};

#endif
