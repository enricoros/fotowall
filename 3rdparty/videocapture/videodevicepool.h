/*
    videodevicepool.h  -  Multiple Video Device Handler Class

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

#ifndef VIDEODEVICEPOOL_H
#define VIDEODEVICEPOOL_H

#include <iostream>


#include "videoinput.h"
#include "videodevicemodelpool.h"
#include <qstring.h>
#include <qimage.h>
#include <q3valuevector.h>
#include <qmutex.h>
#include <kcombobox.h>
#include "videodevice.h"
#include <kconfig.h>
#include <kglobal.h>
#include <solid/device.h>

namespace Phonon {
namespace VideoCapture {

/**
This class allows kopete to check for the existence, open, configure, test, set parameters, grab frames from and close a given video capture card using the Video4Linux API.

@author Cláudio da Silveira Pinheiro
*/

typedef QVector<Phonon::VideoCapture::VideoDevice> VideoDeviceVector;


class  KDE_EXPORT VideoDevicePool : public QObject
{
Q_OBJECT
public:
	static VideoDevicePool* self();
	int open();
	int open(int device);
	bool isOpen();
	int getFrame();
	int width();
	int minWidth();
	int maxWidth();
	int height();
	int minHeight();
	int maxHeight();
	int setSize( int newwidth, int newheight);
	int close();
	int startCapturing();
	int stopCapturing();
	int readFrame();
	int getImage(QImage *qimage);
	int selectInput(int newinput);
	int setInputParameters();
	int scanDevices();
	void registerDevice( Solid::Device & dev );
	bool hasDevices();
	size_t size();
	~VideoDevicePool();
	VideoDeviceVector m_videodevice; // Vector to be filled with found devices
	VideoDeviceModelPool m_modelvector;  // Vector to be filled with unique device models
	int fillDeviceKComboBox(KComboBox *combobox);
	int fillInputKComboBox(KComboBox *combobox);
	int fillStandardKComboBox(KComboBox *combobox);
	int currentDevice();
	int currentInput();
	unsigned int inputs();

	float getBrightness();
	float setBrightness(float brightness);
	float getContrast();
	float setContrast(float contrast);
	float getSaturation();
	float setSaturation(float saturation);
	float getWhiteness();
	float setWhiteness(float whiteness);
	float getHue();
	float setHue(float hue);

	bool getAutoBrightnessContrast();
	bool setAutoBrightnessContrast(bool brightnesscontrast);
	bool getAutoColorCorrection();
	bool setAutoColorCorrection(bool colorcorrection);
	bool getImageAsMirror();
	bool setImageAsMirror(bool imageasmirror);

	void loadConfig(); // Load configuration parameters;
	void saveConfig(); // Save configuretion parameters;

signals:
	/**
	 * Provisional signatures, probably more useful to indicate which device was registered
	 */
	void deviceRegistered( const QString & udi );
	void deviceUnregistered( const QString & udi );
protected slots:
	/**
	 * Slot called when a new device is added to the system
	 */
	void deviceAdded( const QString & udi );
	void deviceRemoved( const QString & udi );
protected:
	int xioctl(int request, void *arg);
	int errnoReturn(const char* s);
	int showDeviceCapabilities(unsigned int device);
	void guessDriver();
	int m_current_device;
	struct imagebuffer m_buffer; // only used when no devices were found

	QMutex m_ready;
private:
	VideoDevicePool();
	static VideoDevicePool* s_self;
	static __u64 m_clients; // Number of instances
};

}
}

#endif // VIDEODEVICEPOOL_H
