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

#ifndef __WordcloudAppliance_h__
#define __WordcloudAppliance_h__

#include "Shared/PlugGui/AbstractAppliance.h"
#include "Wordcloud/Cloud.h"
#include "ui_WordcloudAppliance.h"
class QMenu;
class WordcloudSidebar;

class WordcloudAppliance : public QObject, public PlugGui::AbstractAppliance
{
  Q_OBJECT
public:
  WordcloudAppliance(Wordcloud::Cloud * extCloud, QObject * parent = 0);
  ~WordcloudAppliance();

  // take the cloud (NOTE: DELETE THE OBJECT RIGHT AFTER THIS!)
  Wordcloud::Cloud * takeCloud();

  // peek into the cloud
  Wordcloud::Cloud * cloud() const;

  // ::Appliance::AbstractAppliance
  QString applianceName() const { return tr("Wordcloud"); }
  bool appliancePendingChanges() const;
  bool applianceSave(const QString & filePath = QString());

private:
  Wordcloud::Cloud * m_extCloud;
  AbstractScene * m_scene;
  Ui::WordcloudApplianceElements * ui;
  QWidget * m_dummyWidget;
  WordcloudSidebar * m_sidebar;

private Q_SLOTS:
  void slotRegenCloud();
  void slotRandomizeCloud();
};

#include "Shared/AbstractScene.h"
class WordcloudScene : public AbstractScene
{
  Q_OBJECT
public:
  WordcloudScene(Wordcloud::Cloud * cloud, QObject * parent = 0);

  // ::AbstractScene
  void resize(const QSize & size);

private:
  Wordcloud::Cloud * m_cloud;

private Q_SLOTS:
  void slotWordMoved();
};

#endif
