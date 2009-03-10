/*
    videodevicemodelpool.h  -  Multiple Video Device Model Handler Class

    Copyright (c) 2005-2008 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "videodevicemodelpool.h"

namespace Phonon {
namespace VideoCapture {

VideoDeviceModelPool::VideoDeviceModelPool()
{
}


VideoDeviceModelPool::~VideoDeviceModelPool()
{
}

void VideoDeviceModelPool::clear()
{
	m_devicemodel.clear();
}

size_t VideoDeviceModelPool::size()
{
	return m_devicemodel.size();
}

size_t VideoDeviceModelPool::addModel( QString newmodel )
{
	VideoDeviceModel newdevicemodel;
	newdevicemodel.model=newmodel;
	newdevicemodel.count=0;

	if(m_devicemodel.size())
	{
		for ( int loop = 0 ; loop < m_devicemodel.size(); loop++)
		if (newmodel == m_devicemodel[loop].model)
		{
			kDebug() << "Model " << newmodel << " already exists.";
			m_devicemodel[loop].count++;
			return m_devicemodel[loop].count;
		}
	}
	m_devicemodel.push_back(newdevicemodel);
	m_devicemodel[m_devicemodel.size()-1].model = newmodel;
	m_devicemodel[m_devicemodel.size()-1].count = 0;
	return 0;
}


}
}
