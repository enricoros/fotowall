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

#ifndef __AbstractAppliance_h__
#define __AbstractAppliance_h__

#include "../AbstractScene.h"
#include "Container.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariant>

namespace PlugGui
{

/**
    \brief A collection of widgets that can be plugged into a Container
*/
class AbstractAppliance
{
public:
  // plugging into a container
  bool addToApplianceContainer(Container * container);
  void removeFromApplianceContainer();

  // appliance description and external control
  virtual QString applianceName() const = 0;
  virtual bool applianceCommand(int command);
  virtual bool appliancePendingChanges() const;
  virtual bool applianceSave(const QString & filePath = QString());

  // this checks that the appliance contents have been destroyed
  virtual ~AbstractAppliance();

protected:
  // used by reimpls to access the container
  void windowTitleSet(const QString & title);
  void windowTitleClear();
  void sceneSet(AbstractScene *);
  void sceneClear();
  void topbarAddWidget(QWidget *, bool rightBar = false, int index = -1);
  void topbarRemoveWidget(QWidget *);
  void sidebarSetWidget(QWidget *);
  void sidebarClearWidget();
  void centralwidgetSet(QWidget *);
  void centralwidgetClear();
  void containerValueSet(quint32 key, const QVariant & value);
  void setFocusToScene();

private:
  void updateContainerTopbar();
  void detachFromContainer();

  typedef QPointer<QWidget> WidgetPointer;
  typedef QPointer<AbstractScene> ScenePointer;
  typedef QPointer<Container> ContainerPointer;
  typedef QMap<int, QVariant> ValueMap;

  QString m_windowTitle;
  ScenePointer m_pScene;
  QList<WidgetPointer> m_pTopbar;
  WidgetPointer m_pSidebar;
  WidgetPointer m_pCentral;
  ContainerPointer m_containerPtr;
  ValueMap m_values;
};

} // namespace PlugGui

#endif
