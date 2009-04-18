/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __VideoProvider_h__
#define __VideoProvider_h__

#include "AbstractContent.h"
#include <QList>
class QTimer;
class VideoInput;

/**
    @brief Video broker: initializes all the video inputs and streams pixmaps
*/
class VideoProvider : public QObject
{
    Q_OBJECT
    public:
        /// singleton
        static VideoProvider * instance();
        VideoProvider();
        ~VideoProvider();

        // how many inputs
        int inputCount() const;

        // register to an input
        void connectInput(int input, QObject * receiver, const char * method);
        void disconnectReceiver(QObject * receiver);

    Q_SIGNALS:
        void inputCountChanged(int count);

    private:
        QTimer * m_snapTimer;
        QList<VideoInput *> m_inputs;

    private Q_SLOTS:
        void scanDevices();
        void slotInitVideo(const QString & device);
        void slotCaptureVideoFrames();
};

#if defined(HAS_VIDEOCAPTURE)
namespace VideoCapture { class VideoDevice; }
#endif

/// @internal not in cpp for MOC-ing purpose only
class VideoInput : public QObject
{
    Q_OBJECT
    public:
        VideoInput();
        ~VideoInput();

        bool active;
        QList<QObject *> receivers;
#if defined(HAS_VIDEOCAPTURE)
        VideoCapture::VideoDevice * device;
#endif

    Q_SIGNALS:
        friend class VideoProvider;
        void newPixmap(const QPixmap & pixmap);
};

#endif
