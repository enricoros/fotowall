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

#ifndef __BlinkingToolButton_h__
#define __BlinkingToolButton_h__

#include <QPalette>
#include <QToolButton>

class BlinkingToolButton : public QToolButton
{
  Q_OBJECT
  Q_PROPERTY(QColor backColor READ backColor WRITE setBackColor)
  Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
public:
  BlinkingToolButton(QWidget * parent = 0);

public Q_SLOTS:
  void drawAttenction();
  void clearAttenction();

protected:
  // ::QWidget
  void paintEvent(QPaintEvent * event);
  QSize sizeHint() const;

private:
  QColor backColor() const;
  void setBackColor(const QColor & color);
  QColor textColor() const;
  void setTextColor(const QColor & color);
  qreal markOpacity() const;
  void setMarkOpacity(qreal);

  QPalette m_palette;
  QPixmap m_markPixmap;
  qreal m_markOpacity;
};

#endif
