/*
    VideoInput.h  -  Video Input Class

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>
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

#ifndef __VideoInput_h__
#define __VideoInput_h__

#include <QString>

#ifdef Q_OS_LINUX
#include <asm/types.h>
#undef __STRICT_ANSI__
#endif

#ifndef __u64 //required by videodev.h
#define __u64 quint64
#endif

namespace VideoCapture {

/**
    @brief Describe/Control the parameters of a single video stream
    @author Kopete Developers - modified by Enrico Ros for Fotowall inclusion
*/
class VideoInput{
public:
	VideoInput();
	~VideoInput();

	QString name;
	int  hastuner;
	__u64 m_standards;
	float getBrightness();
	float setBrightness(float brightness);
	float getContrast();
	float setContrast(float contrast);
	float getSaturation();
	float setSaturation(float saturation);
	float getWhiteness();
	float setWhiteness(float whiteness);
	float getHue();
	float setHue(float Hue);
	bool getAutoBrightnessContrast();
	bool setAutoBrightnessContrast(bool brightnesscontrast);
	bool getAutoColorCorrection();
	bool setAutoColorCorrection(bool colorcorrection);
	bool getImageAsMirror();
	bool setImageAsMirror(bool imageasmirror);

private:
// this block must be changed to use a vector of controls instead of fixed ones
	float m_brightness;
	float m_contrast;
	float m_saturation;
	float m_whiteness;
	float m_hue;

	bool m_autobrightnesscontrast;
	bool m_autocolorcorrection;
	bool m_imageasmirror;
};

}

#endif
