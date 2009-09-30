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

#include "ApplianceManager.h"

#include "AbstractAppliance.h"

using namespace Appliance;

Manager::Manager()
  : m_container(0)
{
}

Manager::~Manager()
{
    if (!m_appliances.isEmpty()) {
        qWarning("Appliance::Manager::~Manager: not empty!");
        clearAppliances();
    }
    m_container = 0;
}

void Manager::setContainer(Container * container)
{
    if (m_container == container)
        return;

    m_container = container;
    if (AbstractAppliance * app = currentAppliance()) {
        if (m_container)
            app->addToContainer(m_container);
        else
            app->removeFromContainer();
    }
}

Container * Manager::container() const
{
    return m_container;
}

void Manager::stackAppliance(AbstractAppliance * appliance)
{
    if (!appliance)
        return;
    if (AbstractAppliance * app = currentAppliance())
        app->removeFromContainer();
    m_appliances.append(appliance);
    if (m_container)
        appliance->addToContainer(m_container);
    emit structureChanged();
}

QList<AbstractAppliance *> Manager::stackedAppliances() const
{
    return m_appliances;
}

AbstractAppliance * Manager::currentAppliance() const
{
    return m_appliances.isEmpty() ? 0 : m_appliances.last();
}

void Manager::popAppliance()
{
    // delete last
    bool removed = false;
    if (!m_appliances.isEmpty()) {
        AbstractAppliance * app = m_appliances.takeLast();
        if (m_container)
            app->removeFromContainer();
        // TODO - SAVE/SERIALIZE/OTHER LINK HERE
        delete app;
        removed = true;
    }

    // show the last-1
    if (m_container && !m_appliances.isEmpty())
        m_appliances.last()->addToContainer(m_container);

    // notify about the change
    if (removed)
        emit structureChanged();
}

void Manager::clearAppliances()
{
    while (!m_appliances.isEmpty())
        popAppliance();
}
