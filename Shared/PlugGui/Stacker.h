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

#ifndef __Stacker_h__
#define __Stacker_h__

#include <QList>
#include <QObject>

namespace PlugGui
{

class Container;
class AbstractAppliance;

class Stacker
{
public:
  Stacker();
  virtual ~Stacker();

  // set container (runtime changeable)
  void setContainer(Container * container);
  Container * container() const;

  // appliance stacking operations
  virtual void stackAppliance(AbstractAppliance * appliance);
  virtual QList<AbstractAppliance *> stackedAppliances() const;
  virtual AbstractAppliance * currentAppliance() const;
  virtual int applianceCount() const;
  virtual void popAppliance();
  virtual void clearAppliances();

protected:
  // notify (ex. on a push/pop operation)
  virtual void structureChanged();

private:
  Container * m_container;
  QList<AbstractAppliance *> m_appliances;
};

} // namespace PlugGui

#endif
