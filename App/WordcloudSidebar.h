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

#ifndef __WordcloudSidebar_h__
#define __WordcloudSidebar_h__

#include "ui_WordcloudSidebar.h"
#include <QWidget>

class WordcloudSidebar : public QWidget, public Ui::WordcloudSidebar
{
  Q_OBJECT
public:
  WordcloudSidebar(QWidget * parent = 0);
  ~WordcloudSidebar();

private:
  friend class WordcloudAppliance;
};

#endif
