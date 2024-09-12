/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Enrico Ros <enrico.ros@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __HighLightItem_h__
#define __HighLightItem_h__

#include <QBasicTimer>
#include <QGraphicsItem>
#include <QObject>

class HighlightItem : public QObject, public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
public:
  HighlightItem(QGraphicsItem * parent = 0);

  // normalized position
  void setPos(double x, double y);
  void setPosF(double xn, double yn);
  void reposition(const QRectF & rect = QRectF());

  // dispose item after next animation
  void deleteAfterAnimation();

  // ::QGraphicsItem
  QRectF boundingRect() const;
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

  // ::QObject
  void timerEvent(QTimerEvent * event);

private:
  QRectF parentRect() const;

  // normalized position
  bool m_unset;
  double m_xn;
  double m_yn;

  // animation
  QBasicTimer m_timer;
  int m_phase;
  double m_radius;
  bool m_closing;
};

#endif
