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

#include "WordcloudSidebar.h"

#include <QPalette>

WordcloudSidebar::WordcloudSidebar(QWidget * parent) : QWidget(parent)
{
  setupUi(this);

  setAutoFillBackground(true);
  QPalette pal;
  pal.setBrush(QPalette::Window, Qt::lightGray);
  setPalette(pal);
}

WordcloudSidebar::~WordcloudSidebar() {}
