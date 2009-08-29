/*
    VideoInput.cpp  -  Video Input Class

    Copyright (c) 2005-2008 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
    Copyright (c) 2009 - Enrico Ros - Fotowall inclusion <enrico.ros@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "VideoInput.h"

// uncomment following to enable debugging
#define VIDEOINPUT_DEBUG

#ifdef VIDEOINPUT_DEBUG
#define HERE qWarning("%s:%d: %s", __FILE__, __LINE__, __FUNCTION__)
#else
#define HERE
#endif

namespace VideoCapture {

VideoInput::VideoInput()
{
	HERE;
	m_brightness = 0.5;
	m_contrast = 0.5;
	m_saturation = 0.5;
	m_hue = 0.5;
	m_autobrightnesscontrast = false;
	m_autocolorcorrection = false;
}


VideoInput::~VideoInput()
{
	HERE;
}

float VideoInput::getBrightness()
{
	//HERE;
	return m_brightness;
}

float VideoInput::setBrightness(float brightness)
{
	//HERE;
	if ( brightness > 1 )
		brightness = 1;
	else
	if ( brightness < 0 )
		brightness = 0;
	m_brightness = brightness;
	return getBrightness();
}

float VideoInput::getContrast()
{
	//HERE;
	return m_contrast;
}

float VideoInput::setContrast(float contrast)
{
	//HERE;
	if ( contrast > 1 )
		contrast = 1;
	else
	if ( contrast < 0 )
		contrast = 0;
	m_contrast = contrast;
	return getContrast();
}

float VideoInput::getSaturation()
{
	//HERE;
	return m_saturation;
}

float VideoInput::setSaturation(float saturation)
{
	//HERE;
	if ( saturation > 1 )
		saturation = 1;
	else
	if ( saturation < 0 )
		saturation = 0;
	m_saturation = saturation;
	return getSaturation();
}

float VideoInput::getWhiteness()
{
	//HERE;
	return m_whiteness;
}

float VideoInput::setWhiteness(float whiteness)
{
	//HERE;
	if ( whiteness > 1 )
		whiteness = 1;
	else
	if ( whiteness < 0 )
		whiteness = 0;
	m_whiteness = whiteness;
	return getWhiteness();
}

float VideoInput::getHue()
{
	//HERE;
	return m_hue;
}

float VideoInput::setHue(float hue)
{
	//HERE;
	if ( hue > 1 )
		hue = 1;
	else
	if ( hue < 0 )
		hue = 0;
	m_hue = hue;
	return getHue();
}


bool VideoInput::getAutoBrightnessContrast()
{
	//HERE;
	return m_autobrightnesscontrast;
}

bool VideoInput::setAutoBrightnessContrast(bool brightnesscontrast)
{
	//HERE;
	m_autobrightnesscontrast = brightnesscontrast;
	return getAutoBrightnessContrast();
}

bool VideoInput::getAutoColorCorrection()
{
	//HERE;
	return m_autocolorcorrection;
}

bool VideoInput::setAutoColorCorrection(bool colorcorrection)
{
	//HERE;
	m_autocolorcorrection = colorcorrection;
	return getAutoColorCorrection();
}

bool VideoInput::getImageAsMirror()
{
	//HERE;
	return m_imageasmirror;
}

bool VideoInput::setImageAsMirror(bool imageasmirror)
{
	//HERE;
	m_imageasmirror = imageasmirror;
	return getImageAsMirror();
}

}
