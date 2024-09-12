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

#ifndef __HelpAppliance_h__
#define __HelpAppliance_h__

#include "Shared/PlugGui/AbstractAppliance.h"
#include "ui_HelpAppliance.h"
class HelpScene;

class HelpAppliance : public QObject, public PlugGui::AbstractAppliance
{
  Q_OBJECT
public:
  HelpAppliance(QObject * parent = 0);
  ~HelpAppliance();

  // ::Appliance::AbstractAppliance
  QString applianceName() const { return tr("Help"); }

private:
  Ui::HelpApplianceWidgets ui;
  HelpScene * m_helpScene;
  QWidget * m_dummyWidget;

private Q_SLOTS:
  void slotClose();
};

#endif
