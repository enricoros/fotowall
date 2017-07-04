/*
    VideoDevice.cpp  -  Video Device Low-level Support

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

#ifndef __VideoDevice_h__
#define __VideoDevice_h__

#include <QtGlobal>
#include <QSize>
#include <QString>
#include <QVector>
#include "VideoInput.h"
class QImage;

// check which backends to enable, based on the environment
#if defined(Q_OS_LINUX)

    // this is here to detect wether V4L2 is present
    #include <asm/types.h>
    #undef __STRICT_ANSI__
    #ifndef __u64 // required by videodev.h
    #define __u64 quint64
    #endif
    #ifndef __s64 // required by videodev.h
    #define __s64 qint64
    #endif
    #ifndef pgoff_t
    #define pgoff_t unsigned long
    #endif
    #include <linux/fs.h>
    #include <linux/kernel.h>
    #include <linux/videodev2.h>
    #ifdef V4L2_CAP_VIDEO_CAPTURE
    #define VD_BUILD_LINUX_V4L2                 ///< build V4L2 code path
    #endif

#elif defined(Q_OS_WIN) && 0 // NOTE: Doesn't currently work, so disable this

    #include <qt_windows.h>
    class QWidget;
    #define VD_BUILD_WIN_VFW                    ///< build VFW code

#endif

namespace VideoCapture {

/**
    \brief One per video grabbing device, can choose which input to stream.
    \author Kopete Developers - modified by Enrico Ros for Fotowall inclusion
    \class VideoDevice
*/
typedef enum {
    // Packed RGB formats
    PIXELFORMAT_NONE	= 0,
    PIXELFORMAT_GREY	= (1 << 0),
    PIXELFORMAT_RGB332	= (1 << 1),
    PIXELFORMAT_RGB444	= (1 << 2),
    PIXELFORMAT_RGB555	= (1 << 3),
    PIXELFORMAT_RGB565	= (1 << 4),
    PIXELFORMAT_RGB555X	= (1 << 5),
    PIXELFORMAT_RGB565X	= (1 << 6),
    PIXELFORMAT_BGR24	= (1 << 7),
    PIXELFORMAT_RGB24	= (1 << 8),
    PIXELFORMAT_BGR32	= (1 << 9),
    PIXELFORMAT_RGB32	= (1 << 10),

    // Bayer RGB format
    PIXELFORMAT_SBGGR8	= (1 << 11),

    // YUV formats
    PIXELFORMAT_YUYV	= (1 << 12),
    PIXELFORMAT_UYVY	= (1 << 13),
    PIXELFORMAT_YUV420P	= (1 << 14),
    PIXELFORMAT_YUV422P	= (1 << 15),

    // Compressed formats
    PIXELFORMAT_JPEG	= (1 << 16),
    PIXELFORMAT_MPEG	= (1 << 17),

    // Reserved formats
    PIXELFORMAT_DV	= (1 << 18),
    PIXELFORMAT_ET61X251= (1 << 19),
    PIXELFORMAT_HI240	= (1 << 20),
    PIXELFORMAT_HM12	= (1 << 21),
    PIXELFORMAT_MJPEG	= (1 << 22),
    PIXELFORMAT_PWC1	= (1 << 23),
    PIXELFORMAT_PWC2	= (1 << 24),
    PIXELFORMAT_SN9C10X	= (1 << 25),
    PIXELFORMAT_WNVA	= (1 << 26),
    PIXELFORMAT_YYUV	= (1 << 27)
    //PIXELFORMAT_ALL	= 0x00003FFF
} PixelFormat;

enum DeviceAttribute {
    DA_None         = 0x0000,
    DA_Capture      = 0x0001,
    DA_ChromaKey    = 0x0002,
    DA_Scale        = 0x0004,
    DA_Overlay      = 0x0008,
    DA_IORead       = 0x0010,
    DA_IOAsync      = 0x0020,
    DA_IOStream     = 0x0040
};

struct DeviceInfo {
    QString prettyName;
    QString version;
    QString filePath;
    int index;
};

enum FrameOperation {
    FO_None                     = 0x0000,
    FO_AutoBrightnessContrast   = 0x0001,
    FO_AutoColorCorrection      = 0x0002,
    FO_HorizontalMirror         = 0x0004
};

struct ImageBuffer {
    QSize size;
    PixelFormat pixelformat;
    QByteArray data;

    ImageBuffer() : pixelformat(PIXELFORMAT_NONE) {}
};

class VideoDevice {
    public:
        VideoDevice(const DeviceInfo & info);
        ~VideoDevice();

        // describe candidate video devices
        static QList<DeviceInfo> scanDevices();

        // open the device for using it
        bool open();
        void close();

        /* the following are valid only when opened */

        bool testAttributes(quint32 attributes) const;
        void printDeviceProperties() const;
        int inputCount() const;
        QSize minSize() const;
        QSize maxSize() const;

        int currentInput() const;
        bool setCurrentInput(int index);

        QSize captureSize() const;
        bool setCaptureSize(const QSize & newSize);

        bool startCapturing();
        bool stopCapturing();

        /* the following are valid only when capturing */

        bool captureFrame();
        bool getLastFrame(QImage * qImage) const;

        quint32 frameOperations() const;
        void setFrameOperations(quint32 operations);

    private:
        bool queryDeviceProperties();
        void detectPixelFormats();

        PixelFormat setPixelFormat(PixelFormat newformat) const;
        int pixelFormatToPlatform(PixelFormat pixelformat) const;
        PixelFormat pixelFormatFromPlatform(int platform) const;
        int pixelFormatDepth(PixelFormat pixelformat) const;
        QString pixelFormatName(PixelFormat pixelformat) const;
        QString pixelFormatNamePlatform(int pixelformat) const;

        bool setInputParameters();

    private:
        DeviceInfo m_info;
        quint32 m_frameOperations;

        // read on open
        quint32 m_attributes;
        QSize m_minSize;
        QSize m_maxSize;
        QVector<VideoCapture::VideoInput> m_inputs;

        // status
        int m_currentInput;
        ImageBuffer m_imageBuffer;
        bool m_capturing;

#if defined(VD_BUILD_LINUX_V4L2)
        // Video 4 Linux data
        int m_videoFileDescriptor;
        enum Driver {
            LINUX_DRIVER_NONE,
            LINUX_DRIVER_V4L2
        } m_linuxDriver;
        enum IOMethod {
            IO_METHOD_NONE,
            IO_METHOD_READ,
            IO_METHOD_MMAP,
            IO_METHOD_USERPTR
        } m_linuxIO;
        struct FrameMemory {
            uchar * start;
            size_t length;
        };
        QVector<FrameMemory> m_frameMemory;
        bool detectSignalStandards() const;
        int xioctl(int request, void *arg) const;
        bool initIoRead();
        bool initIoMmap();
        bool initIoUserptr();
        void enumerateControls() const;
        void enumerateMenu(quint32 id, quint32 min, quint32 max) const;
#elif defined(VD_BUILD_WIN_VFW)
        // Video For Windows data
        QWidget * m_vWidget;
        HWND m_vHwnd;
#endif
};

}

#endif
