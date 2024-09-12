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

#ifndef __SelectionProperties_h__
#define __SelectionProperties_h__

#include "AbstractContent.h"
#include <QList>
#include <QWidget>
class PictureContent;
class TextContent;

class SelectionProperties : public QWidget
{
  Q_OBJECT
public:
  SelectionProperties(QList<AbstractContent *> selection, QWidget * parent = 0);
  ~SelectionProperties();

Q_SIGNALS:
  void collateSelection();
  void deleteSelection();

private:
  QList<AbstractContent *> m_content;
  QList<PictureContent *> m_pictures;
  QList<TextContent *> m_texts;
};

#endif
