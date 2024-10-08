/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __StyledButtonItem_h__
#define __StyledButtonItem_h__

#include <QFont>
#include <QGraphicsItem>

class StyledButtonItem : public QObject, public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
public:
  StyledButtonItem(const QString & text, const QFont & font, QGraphicsItem * parent = 0);

  QString text() const;
  void setText(const QString & text);

  // ::QGraphicsItem
  QRectF boundingRect() const;
  void keyPressEvent(QKeyEvent * event);
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

Q_SIGNALS:
  void clicked();

protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
  void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

private:
  void drawButton(bool pressed, bool hovered, QPainter * painter);
  QString m_text;
  QFont m_font;
  QRectF m_rect;
  bool m_hovered;
  bool m_pressed;
};

#endif
