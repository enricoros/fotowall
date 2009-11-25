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

// enable frameworks
#if defined(Q_OS_LINUX)
    #define VIDEODEV_LINUX_V4L
    // this is here to detect wether V4L2 is present
    #include <asm/types.h>
    #undef __STRICT_ANSI__
    #ifndef __u64 //required by videodev.h
    #define __u64 quint64
    #endif
    #ifndef __s64 //required by videodev.h
    #define __s64 qint64
    #endif
    #ifndef pgoff_t
    #define pgoff_t unsigned long
    #endif
    #include <linux/fs.h>
    #include <linux/kernel.h>
    #include <linux/videodev.h>
    #ifdef V4L2_CAP_VIDEO_CAPTURE
        #define VIDEODEV_LINUX_V4L_TWO
    #endif
#elif defined(Q_WS_WIN)
    #define VIDEODEV_WIN_VFW
#endif

namespace VideoCapture {

/**
    @brief One video grabbing device, can stream multiple inputs.
    @author Kopete Developers - modified by Enrico Ros for Fotowall inclusion
    @class VideoDevice
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

/*typedef enum {
    // One bit for each
    STANDARD_PAL_B		= (1 << 0),
    STANDARD_PAL_B1		= (1 << 1),
    STANDARD_PAL_G		= (1 << 2),
    STANDARD_PAL_H		= (1 << 3),
    STANDARD_PAL_I		= (1 << 4),
    STANDARD_PAL_D		= (1 << 5),
    STANDARD_PAL_D1		= (1 << 6),
    STANDARD_PAL_K		= (1 << 7),

    STANDARD_PAL_M		= (1 << 8),
    STANDARD_PAL_N		= (1 << 9),
    STANDARD_PAL_Nc		= (1 << 10),
    STANDARD_PAL_60		= (1 << 11),
    // STANDARD_PAL_60 is a hybrid standard with 525 lines, 60 Hz refresh rate, and PAL color modulation with a 4.43 MHz color subcarrier. Some PAL video recorders can play back NTSC tapes in this mode for display on a 50/60 Hz agnostic PAL TV.

    STANDARD_NTSC_M		= (1 << 12),
    STANDARD_NTSC_M_JP	= (1 << 13),
    STANDARD_NTSC_443	= (1 << 14),
    // STANDARD_NTSC_443 is a hybrid standard with 525 lines, 60 Hz refresh rate, and NTSC color modulation with a 4.43 MHz color subcarrier.
    STANDARD_NTSC_M_KR	= (1 << 15),

    STANDARD_SECAM_B	= (1 << 16),
    STANDARD_SECAM_D	= (1 << 17),
    STANDARD_SECAM_G	= (1 << 18),
    STANDARD_SECAM_H	= (1 << 19),
    STANDARD_SECAM_K	= (1 << 20),
    STANDARD_SECAM_K1	= (1 << 21),
    STANDARD_SECAM_L	= (1 << 22),
    STANDARD_SECAM_LC	= (1 << 23),

    // ATSC/HDTV
    STANDARD_ATSC_8_VSB	= (1 << 24),
    STANDARD_ATSC_16_VSB	= (1 << 25),

    // Some common needed stuff
    STANDARD_PAL_BG		= ( STANDARD_PAL_B   | STANDARD_PAL_B1   | STANDARD_PAL_G  ),
    STANDARD_PAL_DK		= ( STANDARD_PAL_D   | STANDARD_PAL_D1   | STANDARD_PAL_K  ),
    STANDARD_PAL		= ( STANDARD_PAL_BG  | STANDARD_PAL_DK   | STANDARD_PAL_H    | STANDARD_PAL_I  ),
    STANDARD_NTSC		= ( STANDARD_NTSC_M  | STANDARD_NTSC_M_JP| STANDARD_NTSC_M_KR ),
    STANDARD_SECAM_DK       = ( STANDARD_SECAM_D | STANDARD_SECAM_K  | STANDARD_SECAM_K1 ),
    STANDARD_SECAM		= ( STANDARD_SECAM_B | STANDARD_SECAM_G  | STANDARD_SECAM_H  | STANDARD_SECAM_DK | STANDARD_SECAM_L | STANDARD_SECAM_LC ),

    // some merged standards
    STANDARD_MN		= ( STANDARD_PAL_M  | STANDARD_PAL_N    | STANDARD_PAL_Nc  | STANDARD_NTSC ),
    STANDARD_B		= ( STANDARD_PAL_B  | STANDARD_PAL_B1   | STANDARD_SECAM_B ),
    STANDARD_GH		= ( STANDARD_PAL_G  | STANDARD_PAL_H    | STANDARD_SECAM_G | STANDARD_SECAM_H ),
    STANDARD_DK		= ( STANDARD_PAL_DK | STANDARD_SECAM_DK ),

    STANDARD_525_60		= ( STANDARD_PAL_M   | STANDARD_PAL_60   | STANDARD_NTSC     | STANDARD_NTSC_443),
    STANDARD_625_50		= ( STANDARD_PAL     | STANDARD_PAL_N    | STANDARD_PAL_Nc   | STANDARD_SECAM),
    STANDARD_ATSC		= ( STANDARD_ATSC_8_VSB | STANDARD_ATSC_16_VSB ),

    STANDARD_UNKNOWN	= 0,
    STANDARD_ALL		= ( STANDARD_525_60  | STANDARD_625_50)
} signal_standard;*/

struct DeviceInfo {
    QString name;
    QString version;
    int index;
};

class VideoDevice {
    public:
        VideoDevice(const DeviceInfo & info);
        ~VideoDevice();

        static QList<DeviceInfo> devices();

        // OK
        bool open();
        void close();
        bool isOpen() const;
        bool printDeviceProperties() const;

        int inputCount() const;
        int currentInput() const;
        bool setCurrentInput(int index);

        int minWidth() const;
        int minHeight() const;
        int maxWidth() const;
        int maxHeight() const;

        bool setCaptureSize(int newWidth, int newHeight);
        QSize captureSize() const;
        bool startCapturing();
        bool stopCapturing();

        bool acquireFrame();
        bool getImage(QImage *qimage) const;

    private:
        bool queryDeviceProperties();
        bool detectSignalStandards() const;
        void detectPixelFormats();

        PixelFormat setPixelFormat(PixelFormat newformat) const;
        int pixelFormatToPlatform(PixelFormat pixelformat) const;
        PixelFormat pixelFormatFromPlatform(int platform) const;
        int pixelFormatDepth(PixelFormat pixelformat) const;
        QString pixelFormatName(PixelFormat pixelformat) const;
        QString pixelFormatNamePlatform(int pixelformat) const;

        /*quint64 signalStandardCode(signal_standard standard);
        QString signalStandardName(signal_standard standard) const;
        QString signalStandardName(int standard) const;*/
        bool setInputParameters();
        /*float getBrightness();
        float setBrightness(float brightness);
        float getContrast();
        float setContrast(float contrast);
        float getSaturation();
        float setSaturation(float saturation);
        float getWhiteness();
        float setWhiteness(float whiteness);
        float getHue();
        float setHue(float Hue);*/

        bool getAutoBrightnessContrast() const;
        bool setAutoBrightnessContrast(bool brightnesscontrast);
        bool getAutoColorCorrection() const;
        bool setAutoColorCorrection(bool colorcorrection);
        bool getImageAsMirror();
        bool setImageAsMirror(bool imageasmirror);

        bool canCapture() const;
        bool canChromakey() const;
        bool canScale() const;
        bool canOverlay() const;
        bool canRead() const;
        bool canAsyncIO() const;
        bool canStream() const;

    private:
        DeviceInfo m_info;
#if defined(VIDEODEV_LINUX_V4L)
        int m_videoFileDescriptor;
#endif
        QString m_videoCardName;
        bool m_videocapture;
        bool m_videochromakey;
        bool m_videoscale;
        bool m_videooverlay;
        bool m_videoread;
        bool m_videoasyncio;
        bool m_videostream;

        enum videodev_driver {
            VIDEODEV_DRIVER_NONE
#if defined(VIDEODEV_LINUX_V4L)
          , VIDEODEV_DRIVER_V4L
#endif
#if defined(VIDEODEV_LINUX_V4L_TWO)
          , VIDEODEV_DRIVER_V4L2
#endif
#if defined(VIDEODEV_WIN_VFW)
          , VIDEODEV_DRIVER_AVICAP
#endif
        } m_driver;

        enum io_method {
            IO_METHOD_NONE,
            IO_METHOD_READ,
            IO_METHOD_MMAP,
            IO_METHOD_USERPTR
        } m_ioMethod;

        int m_minWidth, m_minHeight, m_maxWidth, m_maxHeight;

        QVector<VideoCapture::VideoInput> m_input;
        int m_currentInput;

        struct ImageBuffer {
            int width;
            int height;
            PixelFormat pixelformat;
            QByteArray data;
        } m_imageBuffer;

        struct DataBuffer {
            uchar * start;
            size_t length;
        };
        QVector<DataBuffer> m_dataBuffers;

        bool m_capturing;

        bool initIoRead();
        bool initIoMmap();
        bool initIoUserptr();
#if defined(VIDEODEV_LINUX_V4L)
        int xioctl(int request, void *arg) const;
#if defined(VIDEODEV_LINUX_V4L_TWO)
        void enumerateControls() const;
        void enumerateMenu(quint32 id, quint32 min, quint32 max) const;
#endif
#endif
};

}

#endif
