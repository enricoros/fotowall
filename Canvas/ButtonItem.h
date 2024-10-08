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

#ifndef __ButtonItem_h__
#define __ButtonItem_h__

#include <QBrush>
#include <QGraphicsItem>
#include <QIcon>
#include <QPointF>

class ButtonItem : public QObject, public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
public:
  enum Type
  {
    Control,
    FlipV,
    FlipH,
    Rotate
  };
  ButtonItem(Type type, const QBrush & brush, const QIcon & icon, QGraphicsItem * parent);

  Type buttonType() const;
  int width() const;
  int height() const;
  void setSelectsParent(bool selects);

  // ::QGraphicsItem
  QRectF boundingRect() const;
  void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
  void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
  void mousePressEvent(QGraphicsSceneMouseEvent * event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

Q_SIGNALS:
  void dragging(const QPointF & sceneRelPoint, Qt::KeyboardModifiers modifiers);
  void pressed();
  void clicked();
  void doubleClicked();
  void releaseEvent(QGraphicsSceneMouseEvent * event);

private:
  Type m_type;
  QIcon m_icon;
  QBrush m_brush;
  bool m_selectsParent;
  QPointF m_startPos;
};

#endif
