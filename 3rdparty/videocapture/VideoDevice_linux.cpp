/*
    VideoDevice.cpp  -  Video Device Low-level Support

    Copyright (c) 2005-2006 by Cláudio da Silveira Pinheiro   <taupter@gmail.com>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <QDebug>

#include "VideoInput.h"
#include "VideoDevice_linux.h"

#include "bayer.h"
#include "sonix_compress.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

// the number of buffer MMAPed or UPTRed
#define IO_DATA_BUFFERS 2

namespace VideoCapture {

VideoDevice::VideoDevice(const QString & fileName)
  : m_videoFileName(fileName)
  , m_videoFileDescriptor(-1)
  , m_driver(VIDEODEV_DRIVER_NONE)
  , m_ioMethod(IO_METHOD_NONE)
  , m_minWidth(-1)
  , m_minHeight(-1)
  , m_maxWidth(-1)
  , m_maxHeight(-1)
  , m_currentInput(-1)
  , m_capturing(false)
{
    // empty image buffer
    m_imageBuffer.width = 0;
    m_imageBuffer.height = 0;
    m_imageBuffer.pixelformat = PIXELFORMAT_NONE;
}

VideoDevice::~VideoDevice()
{
    close();
}

#ifdef V4L2_CAP_VIDEO_CAPTURE
void VideoDevice::enumerateControls() const
{
    // -----------------------------------------------------------------------------------------------------------------
    // This must turn up to be a proper method to check for controls' existence.
    // v4l2_queryctrl may zero the .id in some cases, even if the IOCTL returns EXIT_SUCCESS (tested with a bttv card, when testing for V4L2_CID_AUDIO_VOLUME).
    // As of 6th Aug 2007, according to the V4L2 specification version 0.21, this behavior is undocumented, and the example 1-8 code found at
    // http://www.linuxtv.org/downloads/video4linux/API/V4L2_API/spec/x519.htm fails because of this behavior with a bttv card.
    struct v4l2_queryctrl queryctrl;

    qDebug() << "VideoDevice::enumerateControls: Checking CID controls";
    for (int currentid = V4L2_CID_BASE; currentid < V4L2_CID_LASTP1; currentid++) {
        CLEAR(queryctrl);
        queryctrl.id = currentid;
        if (0 == xioctl(VIDIOC_QUERYCTRL, &queryctrl)) {
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                continue;
            qDebug() << " Control:" << QString::fromLocal8Bit((const char*)queryctrl.name) << " [" << queryctrl.minimum << "..." << queryctrl.maximum << "] /" << queryctrl.step << " (default:" << queryctrl.default_value <<  ")";
            switch (queryctrl.type) {
                case V4L2_CTRL_TYPE_INTEGER:    break;
                case V4L2_CTRL_TYPE_BOOLEAN:    break;
                case V4L2_CTRL_TYPE_MENU:       enumerateMenu(queryctrl.id, queryctrl.minimum, queryctrl.maximum);    break;
                case V4L2_CTRL_TYPE_BUTTON:     break;
                case V4L2_CTRL_TYPE_INTEGER64:  break;
                case V4L2_CTRL_TYPE_CTRL_CLASS: break;
            }
        } else if (errno != EINVAL)
            perror("VIDIOC_QUERYCTRL");
    }

    qDebug() << "VideoDevice::enumerateControls: Checking CID private controls";
    for (int currentid = V4L2_CID_PRIVATE_BASE;; currentid++) {
        CLEAR(queryctrl);
        queryctrl.id = currentid;
        if (0 == xioctl (VIDIOC_QUERYCTRL, &queryctrl)) {
            if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                continue;
            qDebug() << " Control:" << QString::fromLocal8Bit((const char*)queryctrl.name) << " [" << queryctrl.minimum << "..." << queryctrl.maximum << "] /" << queryctrl.step << " (default: " << queryctrl.default_value << ")";
            if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
                enumerateMenu(queryctrl.id, queryctrl.minimum, queryctrl.maximum);
        } else {
            if (errno == EINVAL)
                break;
            perror ("VIDIOC_QUERYCTRL");
        }
    }
}

void VideoDevice::enumerateMenu(quint32 id, quint32 min, quint32 max) const
{
    qDebug() <<  "  Menu items:";
    struct v4l2_querymenu queryMenu;
    CLEAR(queryMenu);
    queryMenu.id = id;
    for (queryMenu.index = min; queryMenu.index <= max; ++queryMenu.index) {
        if (0 == xioctl (VIDIOC_QUERYMENU, &queryMenu))
            qDebug() <<  "  " << QString::fromLocal8Bit((const char*)queryMenu.name);
        else {
            perror("VIDIOC_QUERYMENU");
            break;
        }
    }
}
#endif

bool VideoDevice::open()
{
    if (m_videoFileDescriptor != -1) {
        qDebug() << "VideoDevice::open: Device is already open";
        return true;
    }

    // open video device and check opening
    m_videoFileDescriptor = ::open(QFile::encodeName(m_videoFileName).constData(), O_RDWR, 0);
    if (m_videoFileDescriptor == -1) {
        qDebug() << "VideoDevice::open: Unable to open file" << m_videoFileName << "Err:"<< errno;
        return false;
    }

    // check device (capabilities and stuff)
    if (!queryDeviceProperties()) {
        qDebug() << "VideoDevice::open: File" << m_videoFileName << "could not be opened.";
        close();
        return false;
    }

    // select the first input
    if (!m_input.isEmpty())
        setCurrentInput(0);
    return true;
}

void VideoDevice::close()
{
    if (isOpen()) {
        if (m_capturing)
            stopCapturing();
        if (-1 == ::close(m_videoFileDescriptor))
            perror("close");
    }
    m_videoFileDescriptor = -1;
}

bool VideoDevice::isOpen() const
{
    return m_videoFileDescriptor != -1;
}

bool VideoDevice::printDeviceProperties() const
{
    if (!isOpen())
        return false;
    qDebug() << "Device model: " << m_videoCardName << " capabilities:";
    if (canCapture())
        qDebug() << "    Video capture";
    if (canRead())
        qDebug() << "        Read";
    if (canAsyncIO())
        qDebug() << "        Asynchronous input/output";
    if (canStream())
        qDebug() << "        Streaming";
    if (canChromakey())
        qDebug() << "    Video chromakey";
    if (canScale())
        qDebug() << "    Video scales";
    if (canOverlay())
        qDebug() << "    Video overlay";
    //qDebug() << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios;
    qDebug() << "    Max res: " << maxWidth() << " x " << maxHeight();
    qDebug() << "    Min res: " << minWidth() << " x " << minHeight();
    qDebug() << "    Inputs : " << inputCount();
    for (int loop=0; loop < inputCount(); ++loop)
        qDebug() << "Input " << loop << ": " << m_input[loop].name << " (tuner: " << m_input[loop].hastuner << ")";
    return true;
}

int VideoDevice::inputCount() const
{
    return m_input.size();
}

int VideoDevice::currentInput() const
{
    return isOpen() ? m_currentInput : 0;
}

bool VideoDevice::setCurrentInput(int index)
{
    if (index < 0 || index >= inputCount())
        return false;
    if (index == m_currentInput)
        return true;
    if (!isOpen()) {
        qWarning("VideoDevice::selectInput: device not opened");
        return false;
    }

    // select the input
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            if (-1 == ioctl(m_videoFileDescriptor, VIDIOC_S_INPUT, &index)) {
                perror ("VIDIOC_S_INPUT");
                return false;
            }
            break;
#endif
        case VIDEODEV_DRIVER_V4L:
            struct video_channel V4L_input;
            CLEAR(V4L_input);
            V4L_input.channel = index;
            V4L_input.norm = 4; // Hey, it's plain wrong! It should be input's signal standard!
            if (-1 == ioctl(m_videoFileDescriptor, VIDIOCSCHAN, &V4L_input)) {
                perror("VIDIOCSCHAN");
                return false;
            }
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }
    m_currentInput = index;
    qDebug() << "VideoDevice::selectInput: selected input" << index << m_input[index].name;

    // apply parameters to the input
    setInputParameters();
    return true;
}

int VideoDevice::minWidth() const
{
    return m_minWidth;
}

int VideoDevice::minHeight() const
{
    return m_minHeight;
}

int VideoDevice::maxWidth() const
{
    return m_maxWidth;
}

int VideoDevice::maxHeight() const
{
    return m_maxHeight;
}

bool VideoDevice::setCaptureSize(int newWidth, int newHeight)
{
    if (!isOpen()) {
        qWarning("VideoDevice::setCaptureSize: not opened");
        return false;
    }
    if (m_capturing) {
        qWarning("VideoDevice::setCaptureSize: can't chance parameters on-the-fly while capturing");
        return false;
    }

    // 1. set the pixel format
    //   this should not be there. It must remain in a completely distict place, cause this method should not change the pixelformat.
    //   this shouldn't try to find a suitable pixel format this way. It should use values discovered by - detectPixelFormats() - to choose a valid one.
    PixelFormat pfSequence[] = {
        PIXELFORMAT_YUV422P, PIXELFORMAT_YUYV, PIXELFORMAT_UYVY, PIXELFORMAT_YUV420P,
        PIXELFORMAT_RGB24, PIXELFORMAT_BGR24, PIXELFORMAT_RGB32, PIXELFORMAT_BGR32,
        PIXELFORMAT_SBGGR8, PIXELFORMAT_SN9C10X
    };
    PixelFormat pixelFormat;
    for (unsigned int i = 0; i < (sizeof(pfSequence) / sizeof(PixelFormat)); i++) {
        pixelFormat = setPixelFormat(pfSequence[i]);
        if (pixelFormat != PIXELFORMAT_NONE)
            break;
    }
    if (pixelFormat == PIXELFORMAT_NONE)
        qWarning("VideoDevice::setCaptureSize: can't set a pixel format. trying to proceed anyways");
    
    // 2. keep the size into boundaries
    m_imageBuffer.width = qBound(m_minWidth,  newWidth,  m_maxWidth );
    m_imageBuffer.height = qBound(m_minHeight, newHeight, m_maxHeight);

    // 3. change resolution for the video device
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            struct v4l2_format format;
            CLEAR(format);
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(VIDIOC_G_FMT, &format))
                qDebug() << "VideoDevice::setCaptureSize: VIDIOC_G_FMT failed (" << errno << "). Returned width: " << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << " " << format.fmt.pix.width << "x" << format.fmt.pix.height;
            format.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            format.fmt.pix.width       = m_imageBuffer.width;
            format.fmt.pix.height      = m_imageBuffer.height;
            format.fmt.pix.field       = V4L2_FIELD_ANY;
            if (-1 != xioctl(VIDIOC_S_FMT, &format)) {
                // Note VIDIOC_S_FMT may change width and height.
                m_imageBuffer.width = format.fmt.pix.width;
                m_imageBuffer.height = format.fmt.pix.height;
                qDebug() << "VideoDevice::setCaptureSize: using" << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << "pixel format, size:" << m_imageBuffer.width << "x" << m_imageBuffer.height;
            } else
                qDebug() << "VideoDevice::setCaptureSize: VIDIOC_S_FMT failed (" << errno << "). Pixel format:" << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << "size:" << format.fmt.pix.width << "x" << format.fmt.pix.height << "(" << newWidth << "x" << newHeight << ")";
            break;
#endif
        case VIDEODEV_DRIVER_V4L: {
            struct video_window V4L_videowindow;
            if (xioctl(VIDIOCGWIN, &V4L_videowindow)== -1) {
                perror ("ioctl VIDIOCGWIN");
                //return false;
            }
            V4L_videowindow.width  = m_imageBuffer.width;
            V4L_videowindow.height = m_imageBuffer.height;
            V4L_videowindow.clipcount = 0;
            if (xioctl(VIDIOCSWIN, &V4L_videowindow)== -1) {
                perror ("ioctl VIDIOCSWIN");
                //					return (NULL);
            }
            qDebug() << "VideoDevice::setCaptureSize: width:" << V4L_videowindow.width << " Height:" << V4L_videowindow.height << " Clipcount:" << V4L_videowindow.clipcount;
            //qDebug() << "libkopete (avdevice): V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth;
            /*if (-1 == xioctl(VIDIOCGFBUF,&V4L_videobuffer))
            qDebug() << "libkopete (avdevice): VIDIOCGFBUF failed (" << errno << "): Device cannot stream";*/
            } break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }

    // 4. setup imagebuffer
    int bitPerPixel = pixelFormatDepth(pixelFormat);
    // simple hack to try to support unknown formats
    if (bitPerPixel < 1) {
        qDebug("VideoDevice::setCaptureSize: invalid depth %d, trying to assume 32", bitPerPixel);
        bitPerPixel = 32;
    }
    int bufferSize = m_imageBuffer.width * m_imageBuffer.height * bitPerPixel / 8;
    if (bufferSize <= 0) {
        qDebug("VideoDevice::setCaptureSize: invalid buffer size %d", bufferSize);
        return false;
    }
    //qDebug() << "VideoDevice::setCaptureSize: buffer size:" << bufferSize << bitPerPixel;
    m_imageBuffer.pixelformat = pixelFormat;
    m_imageBuffer.data.resize(bufferSize);
    return true;
}

QSize VideoDevice::captureSize() const
{
    return QSize(m_imageBuffer.width, m_imageBuffer.height);
}

bool VideoDevice::startCapturing()
{
    // TODO: check that capture size has been set
    if (!isOpen()) {
        qDebug("VideoDevice::startCapturing: not opened");
        return false;
    }

    switch (m_ioMethod) {
        case IO_METHOD_NONE:
            // Device cannot capture frames
            return false;

        case IO_METHOD_READ:
            // init READ i/o
            if (!initIoRead())
                return false;
            break;

        case IO_METHOD_MMAP: {
            // init MMAP i/o
            if (!initIoMmap())
                return false;

#ifdef V4L2_CAP_VIDEO_CAPTURE
            // queue buffers
            for (int i = 0; i < m_dataBuffers.size(); ++i) {
                struct v4l2_buffer buf;
                CLEAR(buf);
                buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index  = i;
                if (-1 == xioctl (VIDIOC_QBUF, &buf)) {
                    perror("VIDIOC_QBUF (mmap)");
                    return false;
                }
            }

            // start streaming
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(VIDIOC_STREAMON, &type)) {
                perror("VIDIOC_STREAMON (uptr)");
                return false;
            }
#endif
            } break;

        case IO_METHOD_USERPTR: {
            // init UPTR i/o
            if (!initIoUserptr())
                return false;

#ifdef V4L2_CAP_VIDEO_CAPTURE
            // queue buffers
            for (int i = 0; i < m_dataBuffers.size(); ++i) {
                struct v4l2_buffer buf;
                CLEAR(buf);
                buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory    = V4L2_MEMORY_USERPTR;
                buf.m.userptr = (unsigned long)m_dataBuffers[i].start;
                buf.length    = m_dataBuffers[i].length;
                if (-1 == xioctl (VIDIOC_QBUF, &buf)) {
                    perror("VIDIOC_QBUF (uptr)");
                    return false;
                }
            }

            // start streaming
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl (VIDIOC_STREAMON, &type)) {
                perror("VIDIOC_STREAMON (uptr)");
                return false;
            }
#endif
            } break;
    }
    qDebug("VideoDevice::startCapturing: started");
    m_capturing = true;
    return true;
}

bool VideoDevice::stopCapturing()
{
    if (!isOpen()) {
        qDebug("VideoDevice::stopCapturing: not opened");
        return false;
    }
    if (!m_capturing) {
        qDebug("VideoDevice::stopCapturing: already stopped");
        return true;
    }

    switch (m_ioMethod) {
        case IO_METHOD_NONE:
            // Device cannot capture frames
            break;

        case IO_METHOD_READ:
            // Nothing to do
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR: {
#ifdef V4L2_CAP_VIDEO_CAPTURE
            // stop streaming
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(VIDIOC_STREAMOFF, &type))
                perror("VIDIOC_STREAMOFF");

            // un-mmap/free userspace buffers
            if (m_ioMethod == IO_METHOD_MMAP) {
                for (int i = 0; i < m_dataBuffers.size(); ++i) {
                    if (::munmap(m_dataBuffers[i].start, m_dataBuffers[i].length) != 0)
                        qDebug() << "VideoDevice::stopCapturing: unable to munmap";
                }
            } else if (m_ioMethod == IO_METHOD_USERPTR) {
                for (int i = 0; i < m_dataBuffers.size(); ++i)
                    ::free(m_dataBuffers[i].start);
            }

            // release device buffers
            struct v4l2_requestbuffers req;
            CLEAR(req);
            req.count  = 0; // this is the key for releasing the buffers
            req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            req.memory = (m_ioMethod == IO_METHOD_MMAP) ? V4L2_MEMORY_MMAP : V4L2_MEMORY_USERPTR;
            if (-1 == xioctl(VIDIOC_REQBUFS, &req))
                perror("VIDIOC_REQBUFS (stop)");
            m_dataBuffers.clear();
#endif
            } break;
    }
    qDebug("VideoDevice::stopCapturing: stopped");
    m_capturing = false;
    return true;
}

bool VideoDevice::acquireFrame()
{
    if (!m_capturing) {
        qDebug("VideoDevice::acquireFrame: start stream before acquiring frames");
        return false;
    }
    if (!isOpen()) {
        qDebug("VideoDevice::acquireFrame: not opened");
        return false;
    }

    switch (m_ioMethod) {
        case IO_METHOD_NONE:
            // Device cannot capture frames
            return false;

        case IO_METHOD_READ: {
            //qDebug() << "VideoDevice::acquireFrame: using IO_METHOD_READ. File descriptor:" << m_videoFileDescriptor << "Buffer address:" << m_imageBuffer.data.constData() << "Size:" << m_imageBuffer.data.size();
            ssize_t bytesread = ::read(m_videoFileDescriptor, m_imageBuffer.data.data(), m_imageBuffer.data.size());
            if (bytesread < 1) { // must verify this point with ov511 driver.
                qDebug("VideoDevice::acquireFrame: IO_METHOD_READ failed");
                switch (errno) {
                    case EAGAIN:
                        return false;
                    case EIO: // Could ignore EIO, see spec. fall through
                    default: {
                        perror("read");
                        return false;
                    }
                }
            }
            if (bytesread < (int)m_imageBuffer.data.size())
                qDebug() << "VideoDevice::acquireFrame: IO_METHOD_READ returned less bytes (" << bytesread << ") than it was asked for (" << m_imageBuffer.data.size() << ")";
            } break;

        case IO_METHOD_MMAP: {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
            // dequeue a buffer
            struct v4l2_buffer v4l2buffer;
            CLEAR(v4l2buffer);
            v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4l2buffer.memory = V4L2_MEMORY_MMAP;
            if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer)) {
                qDebug() << "VideoDevice::acquireFrame:" << m_videoFileName << "MMAPed getFrame failed";
                switch (errno) {
                    case EAGAIN:
                        qDebug() << "VideoDevice::acquireFrame:" << m_videoFileName << " MMAPed getFrame failed: EAGAIN. Pointer: ";
                        return false;

                    case EIO: // Could ignore EIO, see spec. fall through
                    default:
                        perror("VIDIOC_DQBUF (mmap)");
                        return false;
                }
            }

            // copy data to the ImageBuffer
            memcpy(m_imageBuffer.data.data(), m_dataBuffers[v4l2buffer.index].start, m_imageBuffer.data.size());

            // queue the buffer back
            if (-1 == xioctl(VIDIOC_QBUF, &v4l2buffer)) {
                perror("VIDIOC_QBUF (mmap)");
                return false;
            }
#endif
#endif
            } break;

        case IO_METHOD_USERPTR: {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
            // dequeue a buffer
            struct v4l2_buffer v4l2buffer;
            CLEAR(v4l2buffer);
            v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4l2buffer.memory = V4L2_MEMORY_USERPTR;
            if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer)) {
                switch (errno) {
                    case EAGAIN:
                        return false;
                    case EIO: // Could ignore EIO, see spec. fall through
                    default:
                        perror("VIDIOC_DQBUF (uptr)");
                        return false;
                }
            }

            // find the related DataBuffer
            int i = 0;
            for (; i < m_dataBuffers.size(); ++i)
                if (v4l2buffer.m.userptr == (unsigned long)m_dataBuffers[i].start && v4l2buffer.length == m_dataBuffers[i].length)
                    break;
            if (i >= m_dataBuffers.size()) {
                qWarning("VideoDevice::acquireFrame: can't find the related DataBuffer");
                return false;
            }

            // TODO: copy memory?
            // ...

            // queue the buffer back
            if (-1 == xioctl(VIDIOC_QBUF, &v4l2buffer)) {
                perror("VIDIOC_QBUF (uptr)");
                return false;
            }
#endif
#endif
            } break;
    }
    return true;
}

bool VideoDevice::getImage(QImage * qimage) const
{
    // do NOT delete qimage here, as it is received as a parameter
    const int W = m_imageBuffer.width;
    const int H = m_imageBuffer.height;
    if (qimage->width() != W || qimage->height() != H)
        *qimage = QImage(W, H, QImage::Format_ARGB32);

    //qDebug() << "VideoDevice::getImage: capturing in " << pixelFormatName(m_imageBuffer.pixelformat);
    const char * src = m_imageBuffer.data.constData();
    uchar * imgData = qimage->bits();
    int imgBytes = qimage->numBytes();
    bool decoded = false;
    switch (m_imageBuffer.pixelformat) {
        case PIXELFORMAT_NONE	: break;

        // Packed RGB formats
        case PIXELFORMAT_RGB332	:
            {
                int step=0;
                for (int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = (src[step]>>5<<5)+(src[step]>>5<<2)+(src[step]>>6);
                    imgData[loop+1] = (src[step]>>2<<5)+(src[step]<<3>>5<<2)+(src[step]<<3>>6);
                    imgData[loop+2] = (src[step]<<6)+(src[step]<<6>>2)+(src[step]<<6>>4)+(src[step]<<6>>6);
                    imgData[loop+3] = 255;
                    step++;
                }
                decoded = true;
            }
            break;
        case PIXELFORMAT_RGB444	: break;
        case PIXELFORMAT_RGB555	: break;
        case PIXELFORMAT_RGB565	:
            {
                int step=0;
                for (int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = (src[step]<<3)+(src[step]<<3>>5);
                    imgData[loop+1] = ((src[step+1])<<5)|src[step]>>5;
                    imgData[loop+2] = ((src[step+1])&248)+((src[step+1])>>5);
                    imgData[loop+3] = 255;
                    step+=2;
                }
                decoded = true;
            }
            break;
        case PIXELFORMAT_RGB555X: break;
        case PIXELFORMAT_RGB565X: break;
        case PIXELFORMAT_BGR24	:
            {
                int step=0;
                for(int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = src[step+2];
                    imgData[loop+1] = src[step+1];
                    imgData[loop+2] = src[step];
                    imgData[loop+3] = 255;
                    step+=3;
                }
                decoded = true;
            }
            break;
        case PIXELFORMAT_RGB24	:
            {
                int step=0;
                for(int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = src[step];
                    imgData[loop+1] = src[step+1];
                    imgData[loop+2] = src[step+2];
                    imgData[loop+3] = 255;
                    step+=3;
                }
                decoded = true;
            }
            break;
        case PIXELFORMAT_BGR32	:
            {
                int step=0;
                for(int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = src[step+2];
                    imgData[loop+1] = src[step+1];
                    imgData[loop+2] = src[step];
                    imgData[loop+3] = src[step+3];
                    step+=4;
                }
                decoded = true;
            }
            break;
        case PIXELFORMAT_RGB32	:
            memcpy(imgData, src, m_imageBuffer.data.size());
            break;

        // Bayer RGB format
        case PIXELFORMAT_SBGGR8	:
            {
                unsigned char * d = (unsigned char *)malloc(W * H * 3);
                bayer2rgb24(d, (const unsigned char *)src, W, H);
                int step = 0;
                for(int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = d[step+2];
                    imgData[loop+1] = d[step+1];
                    imgData[loop+2] = d[step];
                    imgData[loop+3] = 255;
                    step+=3;
                }
                free(d);
                decoded = true;
            }
            break;

        // YUV formats
        case PIXELFORMAT_GREY	:
            {
                int step=0;
                for(int loop=0;loop < imgBytes;loop+=4) {
                    imgData[loop]   = src[step];
                    imgData[loop+1] = src[step];
                    imgData[loop+2] = src[step];
                    imgData[loop+3] = 255;
                    step++;
                }
                decoded = true;
            }
            break;
        case PIXELFORMAT_YUYV:
        case PIXELFORMAT_UYVY:
        case PIXELFORMAT_YUV420P:
        case PIXELFORMAT_YUV422P:
            {
                const uchar *yptr, *cbptr, *crptr;
                bool halfheight = false;
                bool packed = false;
                // Adjust algorythm to specific YUV data arrangements.
                if (m_imageBuffer.pixelformat == PIXELFORMAT_YUV420P)
                    halfheight = true;
                if (m_imageBuffer.pixelformat == PIXELFORMAT_YUYV) {
                    yptr = (const uchar *)src;
                    cbptr = yptr + 1;
                    crptr = yptr + 3;
                    packed = true;
                } else if (m_imageBuffer.pixelformat == PIXELFORMAT_UYVY) {
                    cbptr = (const uchar *)src;
                    yptr = cbptr + 1;
                    crptr = cbptr + 2;
                    packed = true;
                } else {
                    yptr = (const uchar *)src;
                    cbptr = yptr + (W*H);
                    crptr = cbptr + (W*H/(halfheight ? 4:2));
                }

                for(int y=0; y<H; y++) {
                    // Decode scanline
                    for(int x=0; x<W; x++) {
                        int c,d,e;

                        if (packed) {
                            c = (yptr[x<<1])-16;
                            d = (cbptr[x>>1<<2])-128;
                            e = (crptr[x>>1<<2])-128;
                        } else {
                            c = (yptr[x])-16;
                            d = (cbptr[x>>1])-128;
                            e = (crptr[x>>1])-128;
                        }

                        int r = (298 * c           + 409 * e + 128)>>8;
                        int g = (298 * c - 100 * d - 208 * e + 128)>>8;
                        int b = (298 * c + 516 * d           + 128)>>8;

                        if (r<0) r=0;   if (r>255) r=255;
                        if (g<0) g=0;   if (g>255) g=255;
                        if (b<0) b=0;   if (b>255) b=255;

                        uint *p = (uint*)qimage->scanLine(y)+x;
                        *p = qRgba(r,g,b,255);
                    }
                    // Jump to next line
                    if (packed) {
                        yptr+=W*2;
                        cbptr+=W*2;
                        crptr+=W*2;
                    } else {
                        yptr+=W;
                        if (!halfheight || y&1)
                        {
                            cbptr+=W/2;
                            crptr+=W/2;
                        }
                    }
                }
                decoded = true;
            }
            break;

        // Compressed formats
        case PIXELFORMAT_JPEG	: break;
        case PIXELFORMAT_MPEG	: break;

        // Reserved formats
        case PIXELFORMAT_DV     : break;
        case PIXELFORMAT_ET61X251:break;
        case PIXELFORMAT_HI240	: break;
        case PIXELFORMAT_HM12	: break;
        case PIXELFORMAT_MJPEG	: break;
        case PIXELFORMAT_PWC1	: break;
        case PIXELFORMAT_PWC2	: break;
        case PIXELFORMAT_SN9C10X:
            {
                unsigned char *s = new unsigned char [W * H];
                unsigned char *d = new unsigned char [W * H * 3];
                sonix_decompress_init();
                sonix_decompress(W, H, (const unsigned char *)src, s);
                bayer2rgb24(d, s, W, H);
                int step=0;
                for(int loop=0;loop < imgBytes;loop+=4)
                {
                    imgData[loop]   = d[step+2];
                    imgData[loop+1] = d[step+1];
                    imgData[loop+2] = d[step];
                    imgData[loop+3] = 255;
                    step+=3;
                }
                delete[] s;
                delete[] d;
                decoded = true;
            }
        case PIXELFORMAT_WNVA	: break;
        case PIXELFORMAT_YYUV	: break;
    }

    // handle undecodable images
    if (!decoded) {
        qimage->fill(0xFF0040A0);
        return false;
    }

    // Proccesses image for automatic Brightness/Contrast/Color correction
    if (getAutoBrightnessContrast() || getAutoColorCorrection()) {
        ///unsigned long long result=0;
        unsigned long long R=0, G=0, B=0, A=0, global=0;
        int Rmax=0, Gmax=0, Bmax=0, Amax=0, globalmax=0;
        int Rmin=255, Gmin=255, Bmin=255, Amin=255, globalmin=255;
        int Rrange=255, Grange=255, Brange=255 /**, Arange=255, globarange=255**/;

        // Finds minimum and maximum intensity for each color component
        for(int loop=0;loop < imgBytes;loop+=4) {
            R+=imgData[loop];
            G+=imgData[loop+1];
            B+=imgData[loop+2];
            //			A+=bits[loop+3];
            if (imgData[loop]   < Rmin) Rmin = imgData[loop];
            if (imgData[loop+1] < Gmin) Gmin = imgData[loop+1];
            if (imgData[loop+2] < Bmin) Bmin = imgData[loop+2];
            //			if (bits[loop+3] < Amin) Amin = bits[loop+3];
            if (imgData[loop]   > Rmax) Rmax = imgData[loop];
            if (imgData[loop+1] > Gmax) Gmax = imgData[loop+1];
            if (imgData[loop+2] > Bmax) Bmax = imgData[loop+2];
            //			if (bits[loop+3] > Amax) Amax = bits[loop+3];
        }
        global = R + G + B;
        // Finds overall minimum and maximum intensity
        if (Rmin > Gmin) globalmin = Gmin; else globalmin = Rmin; if (Bmin < globalmin) globalmin = Bmin;
        if (Rmax > Gmax) globalmax = Rmax; else globalmax = Gmax; if (Bmax > globalmax) globalmax = Bmax;
        // If no color correction should be performed, simply level all the intensities so they're just the same.
        // In fact color correction should use the R, G and B variables to detect color deviation and "bump up" the saturation,
        // but it's computationally more expensive and the current way returns better results to the user.
        if (!getAutoColorCorrection()) {
            Rmin = globalmin ; Rmax = globalmax;
            Gmin = globalmin ; Gmax = globalmax;
            Bmin = globalmin ; Bmax = globalmax;
            //			Amin = globalmin ; Amax = globalmax;
        }
        // Calculates ranges and prevent a division by zero later on.
        Rrange = Rmax - Rmin; if (Rrange == 0) Rrange = 255;
        Grange = Gmax - Gmin; if (Grange == 0) Grange = 255;
        Brange = Bmax - Bmin; if (Brange == 0) Brange = 255;
        //			Arange = Amax - Amin; if (Arange == 0) Arange = 255;

        qDebug() << " R: " << R << " G: " << G << " B: " << B << " A: " << A << " global: " << global <<
                " Rmin: " << Rmin << " Gmin: " << Gmin << " Bmin: " << Bmin << " Amin: " << Amin << " globalmin: " << globalmin <<
                " Rmax: " << Rmax << " Gmax: " << Gmax << " Bmax: " << Bmax << " Amax: " << Amax << " globalmax: " << globalmax ;

        for(int loop=0; loop < imgBytes; loop+=4) {
            imgData[loop]   = (imgData[loop]   - Rmin) * 255 / (Rrange);
            imgData[loop+1] = (imgData[loop+1] - Gmin) * 255 / (Grange);
            imgData[loop+2] = (imgData[loop+2] - Bmin) * 255 / (Brange);
            //			bits[loop+3] = (bits[loop+3] - Amin) * 255 / (Arange);
        }
    }
    return true;
}

bool VideoDevice::queryDeviceProperties()
{
    if (m_videoFileDescriptor == -1) {
        qWarning("VideoDevice::queryDeviceProperties: device not open");
        return false;
    }

    // reset caps
    m_driver = VIDEODEV_DRIVER_NONE;
    m_ioMethod = IO_METHOD_NONE;
    m_videocapture = false;
    m_videochromakey = false;
    m_videoscale = false;
    m_videooverlay = false;
    m_videoread = false;
    m_videoasyncio = false;
    m_videostream = false;

#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    struct v4l2_capability V4L2_capabilities;
    CLEAR(V4L2_capabilities);
    if (-1 != xioctl(VIDIOC_QUERYCAP, &V4L2_capabilities)) {
        // check that is a CaptureDevice
        if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            qDebug() << "VideoDevice::queryDeviceProperties:" << m_videoFileName << "is not a video capture device.";
            return false;
        }
        qDebug() << "VideoDevice::queryDeviceProperties:" << m_videoFileName << "is a V4L2 device.";
        m_driver = VIDEODEV_DRIVER_V4L2;
        m_videocapture = true;
        m_videoCardName = QString::fromLocal8Bit((const char*)V4L2_capabilities.card);

        // find out acquiral mechanism, based on capabilities
        if (V4L2_capabilities.capabilities & V4L2_CAP_READWRITE) {
            m_ioMethod = IO_METHOD_READ;
            m_videoread = true;
        }
        if (V4L2_capabilities.capabilities & V4L2_CAP_STREAMING) {
            m_ioMethod = IO_METHOD_MMAP;
            m_videostream = true;
        }
        if (V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
            m_videoasyncio = true;

        // Detect maximum and minimum resolution supported by the V4L2 device. VIDIOC_ENUM_FRAMESIZES is still experimental.
        struct v4l2_format format;
        CLEAR(format);
        format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(VIDIOC_G_FMT, &format))
            qDebug() << "VideoDevice::queryDeviceProperties: VIDIOC_G_FMT failed (" << errno << ").";
        format.type            = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        format.fmt.pix.width   = 32767;
        format.fmt.pix.height  = 32767;
        format.fmt.pix.field   = V4L2_FIELD_ANY;
        if (-1 == xioctl(VIDIOC_S_FMT, &format)) {
            qDebug() << "VideoDevice::queryDeviceProperties: Detecting maximum size with VIDIOC_S_FMT failed (" << errno << "). Returned max width: " << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << " " << format.fmt.pix.width << "x" << format.fmt.pix.height;
            return false;
        }
        m_maxWidth  = format.fmt.pix.width;
        m_maxHeight = format.fmt.pix.height;

        if (-1 == xioctl(VIDIOC_G_FMT, &format))
            qDebug() << "VideoDevice::queryDeviceProperties: VIDIOC_G_FMT failed (" << errno << ").";
        format.type            = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        format.fmt.pix.width   = 1;
        format.fmt.pix.height  = 1;
        format.fmt.pix.field   = V4L2_FIELD_ANY;
        if (-1 == xioctl(VIDIOC_S_FMT, &format)) {
            qDebug() << "VideoDevice::queryDeviceProperties: Detecting minimum size with VIDIOC_S_FMT failed (" << errno << "). Returned min width: " << format.fmt.pix.width << "x" << format.fmt.pix.height;
            return false;
        }
        m_minWidth  = format.fmt.pix.width;
        m_minHeight = format.fmt.pix.height;

        // Buggy driver paranoia
        /* min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
            fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
            fmt.fmt.pix.sizeimage = min;
        m_buffer_size=fmt.fmt.pix.sizeimage ;*/

        // Recreate the inputs vector
        m_input.clear();
        for (unsigned int iNumber=0; iNumber < 99; ++iNumber) {
            // get input description
            struct v4l2_input videoInput;
            CLEAR(videoInput);
            videoInput.index = iNumber;
            if (xioctl(VIDIOC_ENUMINPUT, &videoInput))
                break;

            // append new VideoInput
            VideoInput input;
            input.name = QString::fromLocal8Bit((const char*)videoInput.name);
            input.hastuner = videoInput.type & V4L2_INPUT_TYPE_TUNER;
            detectSignalStandards();
            input.m_standards = videoInput.std;
            m_input.append(input);
            qDebug() << "VideoDevice::queryDeviceProperties: Input " << iNumber << ": " << input.name << " (tuner: " << ((videoInput.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")";
        }
    } else {
        // V4L-only drivers should return an EINVAL in errno to indicate they cannot handle V4L2 calls. Not every driver is compliant, so
        // it will try the V4L api even if the error code is different than expected.
        qDebug() << "VideoDevice::queryDeviceProperties: " << m_videoFileName << " is not a V4L2 device.";
    }
#endif

    if (m_driver == VIDEODEV_DRIVER_NONE) {
        qDebug() << "VideoDevice::queryDeviceProperties: " << m_videoFileName << " Trying V4L API.";

        // check that is a V4L device
        struct video_capability V4L_capabilities;
        CLEAR(V4L_capabilities);
        if (-1 == xioctl(VIDIOCGCAP, &V4L_capabilities)) {
            perror("VIDIOCGCAP");
            return false;
        }
        qDebug() << "VideoDevice::queryDeviceProperties: " << m_videoFileName << " is a V4L device.";

        // init capabilities and min/max sizes
        m_driver = VIDEODEV_DRIVER_V4L;
        m_videoCardName = QString::fromLocal8Bit((const char*)V4L_capabilities.name);
        m_videocapture = V4L_capabilities.type & VID_TYPE_CAPTURE;
        m_videochromakey = V4L_capabilities.type & VID_TYPE_CHROMAKEY;
        m_videoscale = V4L_capabilities.type & VID_TYPE_SCALES;
        m_videooverlay = V4L_capabilities.type & VID_TYPE_OVERLAY;
        //qDebug() << "libkopete (avdevice):     Inputs : " << V4L_capabilities.channels;
        //qDebug() << "libkopete (avdevice):     Audios : " << V4L_capabilities.audios;
        m_minWidth  = V4L_capabilities.minwidth;
        m_maxWidth  = V4L_capabilities.maxwidth;
        m_minHeight = V4L_capabilities.minheight;
        m_maxHeight = V4L_capabilities.maxheight;

        // use a read i/o method
        m_ioMethod = IO_METHOD_READ;
        m_videoread = true;
        struct video_buffer V4L_videobuffer;
        if (-1 != xioctl(VIDIOCGFBUF, &V4L_videobuffer)) {
            //	m_ioMethod = IO_METHOD_MMAP;
            //	m_videostream = true;
            qDebug() << "    Has got a Streaming interface, but we'll stick to READ for compat";
        }

        // Recreate the inputs vector
        m_input.clear();
        for (int iNumber = 0; iNumber < V4L_capabilities.channels; ++iNumber)
        {
            struct video_channel videoInput;
            CLEAR(videoInput);
            videoInput.channel = iNumber;
            videoInput.norm    = 1;
            if (xioctl(VIDIOCGCHAN, &videoInput))
            {
                VideoInput input;
                input.name = QString::fromLocal8Bit((const char*)videoInput.name);
                input.hastuner = videoInput.flags & VIDEO_VC_TUNER;
                detectSignalStandards();
                m_input.append(input);
                qDebug() << "VideoDevice::queryDeviceProperties: Input " << iNumber << ": " << input.name << " (tuner: " << ((videoInput.flags & VIDEO_VC_TUNER) != 0) << ")";
            }
        }
    }
#endif

    // check if able to read from video
    if (m_ioMethod == IO_METHOD_NONE) {
        qDebug() << "VideoDevice::queryDeviceProperties: found no suitable input/output method for " << m_videoFileName;
        return false;
    }

    // print out supported pixel format/s
    detectPixelFormats();

#ifdef V4L2_CAP_VIDEO_CAPTURE
    // print out the controls supported by the input device
    enumerateControls();

    // Reset cropping to default
    struct v4l2_cropcap cropcap;
    CLEAR(cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(VIDIOC_CROPCAP, &cropcap)) {
        // Errors ignored.
    }
    struct v4l2_crop crop;
    CLEAR(crop);
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; // reset to default
    if (-1 == xioctl(VIDIOC_S_CROP, &crop)) {
        switch (errno)
        {
            case EINVAL: break;  // Cropping not supported.
            default:     break;  // Errors ignored.
        }
    }
#endif

    // device can be used
    return true;
}

bool VideoDevice::detectSignalStandards() const
{
    if (!isOpen()) {
        qWarning("VideoDevice::detectSignalStandards: device not opened.");
        return false;
    }

    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2: {
            // get the index of the current input
            int index = 0;
            if (-1 == xioctl(VIDIOC_G_INPUT, &index)) {
                perror("VIDIOC_G_INPUT");
                return false;
            }

            // describe the current input
            struct v4l2_input input;
            CLEAR(input);
            input.index = index;
            if (-1 == xioctl(VIDIOC_ENUMINPUT, &input)) {
                perror("VIDIOC_ENUM_INPUT");
                return false;
            }
            //qWarning("VideoDevice::detectSignalStandards: input %d is '%s' std: 0x%x", index, input.name, (int)input.std);

            // skip standard matching if reported is unknown
            if (input.std != V4L2_STD_UNKNOWN) {
                // match the input.std to all the device's standards
                struct v4l2_standard standard;
                CLEAR(standard);
                standard.index = 0;
                while (-1 != xioctl(VIDIOC_ENUMSTD, &standard)) {
                    //qDebug() << "SSS" << standard.name;
                    if (standard.id & input.std)
                        qDebug() << "VideoDevice::detectSignalStandards:" << /*signalStandardName(standard.id) <<*/ "(" << standard.id << ")";
                    standard.index++;
                }

                // EINVAL indicates the end of the enumeration, which cannot be empty unless this device falls under the USB exception.
                if (errno != EINVAL || standard.index == 0) {
                    perror("VIDIOC_ENUMSTD");
                    return false;
                }
            }
            }break;
#endif
        case VIDEODEV_DRIVER_V4L:
            // TODO
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }
    return true;
}

void VideoDevice::detectPixelFormats()
{
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            struct v4l2_fmtdesc fmtdesc;
            CLEAR(fmtdesc);
            fmtdesc.index = 0;
            fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            while (true) {
                if (-1 == xioctl(VIDIOC_ENUM_FMT, &fmtdesc))
                    break;
                qDebug("VideoDevice::detectPixelFormats: 0x%x '%s'", (int)fmtdesc.pixelformat, qPrintable(pixelFormatNamePlatform(fmtdesc.pixelformat)));
                fmtdesc.index++;
            }
            //if (fmtdesc.index)
            //    break;
            // fall back to V4L detection
#endif
        case VIDEODEV_DRIVER_V4L:
            // TODO: This thing can be used to detect what pixel formats are supported in a API-independent way, but V4L2 has VIDIOC_ENUM_PIXFMT.
            // The correct thing to do is to isolate these calls and do a proper implementation for V4L and another for V4L2 when this thing will be migrated to a plugin architecture.
            qDebug() << "VideoDevice::detectPixelFormats: supported pixel formats (V4L style):";

            // Packed RGB formats
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB332))	qDebug() << pixelFormatName(PIXELFORMAT_RGB332);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB444))	qDebug() << pixelFormatName(PIXELFORMAT_RGB444);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB555))	qDebug() << pixelFormatName(PIXELFORMAT_RGB555);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB565))	qDebug() << pixelFormatName(PIXELFORMAT_RGB565);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB555X))qDebug() << pixelFormatName(PIXELFORMAT_RGB555X);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB565X))qDebug() << pixelFormatName(PIXELFORMAT_RGB565X);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_BGR24))	qDebug() << pixelFormatName(PIXELFORMAT_BGR24);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB24))	qDebug() << pixelFormatName(PIXELFORMAT_RGB24);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_BGR32))	qDebug() << pixelFormatName(PIXELFORMAT_BGR32);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_RGB32))	qDebug() << pixelFormatName(PIXELFORMAT_RGB32);

            // Bayer RGB format
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_SBGGR8))	qDebug() << pixelFormatName(PIXELFORMAT_SBGGR8);

            // YUV formats
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_GREY))	qDebug() << pixelFormatName(PIXELFORMAT_GREY);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YUYV))	qDebug() << pixelFormatName(PIXELFORMAT_YUYV);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_UYVY))	qDebug() << pixelFormatName(PIXELFORMAT_UYVY);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YUV420P))qDebug() << pixelFormatName(PIXELFORMAT_YUV420P);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YUV422P))qDebug() << pixelFormatName(PIXELFORMAT_YUV422P);

            // Compressed formats
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_JPEG))	qDebug() << pixelFormatName(PIXELFORMAT_JPEG);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_MPEG))	qDebug() << pixelFormatName(PIXELFORMAT_MPEG);

            // Reserved formats
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_DV))		qDebug() << pixelFormatName(PIXELFORMAT_DV);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_ET61X251))qDebug() << pixelFormatName(PIXELFORMAT_ET61X251);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_HI240))	qDebug() << pixelFormatName(PIXELFORMAT_HI240);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_HM12))	qDebug() << pixelFormatName(PIXELFORMAT_HM12);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_MJPEG))	qDebug() << pixelFormatName(PIXELFORMAT_MJPEG);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_PWC1))	qDebug() << pixelFormatName(PIXELFORMAT_PWC1);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_PWC2))	qDebug() << pixelFormatName(PIXELFORMAT_PWC2);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_SN9C10X))qDebug() << pixelFormatName(PIXELFORMAT_SN9C10X);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_WNVA))	qDebug() << pixelFormatName(PIXELFORMAT_WNVA);
            if (PIXELFORMAT_NONE != setPixelFormat(PIXELFORMAT_YYUV))	qDebug() << pixelFormatName(PIXELFORMAT_YYUV);
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }
}

PixelFormat VideoDevice::setPixelFormat(PixelFormat newformat) const
{
    // change the pixel format for the video device
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            // query current format
            struct v4l2_format format;
            CLEAR(format);
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 != xioctl(VIDIOC_G_FMT, &format)) {
                // if we're already at the right format, skip setting it
                if (pixelFormatFromPlatform(format.fmt.pix.pixelformat) == newformat)
                    return newformat;
            }
            else
                perror("VIDIOC_G_FMT");

            // try to set the new format
            format.fmt.pix.pixelformat = pixelFormatToPlatform(newformat);
            if (-1 != xioctl(VIDIOC_S_FMT, &format)) {
                // this "if" (not what is contained within) is a fix for a bug in sn9c102 driver.
                if ((int)format.fmt.pix.pixelformat == pixelFormatToPlatform(newformat))
                    return newformat;
            } else {
                //perror("VIDIOC_S_FMT");
                //qDebug() << "VIDIOC_S_FMT failed (" << errno << "). Returned width: " << pixelFormatName(fmt.fmt.pix.pixelformat) << " " << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height;
            }
            break;
#endif
        case VIDEODEV_DRIVER_V4L: {
            struct video_picture V4L_picture;
            if (-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                qDebug() << "VIDIOCGPICT failed (" << errno << ").";
            //qDebug() << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth;
            V4L_picture.palette = pixelFormatToPlatform(newformat);
            V4L_picture.depth   = pixelFormatDepth(newformat);
            if (-1 == xioctl(VIDIOCSPICT,&V4L_picture)) {
                //qDebug() << "Device seems to not support " << pixelFormatName(newformat) << " format. Fallback to it is not yet implemented.";
            }
            if (-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                perror("VIDIOCGPICT");
            //qDebug() << "V4L_picture.palette: " << V4L_picture.palette << " Depth: " << V4L_picture.depth;
            if (pixelFormatFromPlatform(V4L_picture.palette) == newformat)
                return newformat;
            } break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }
    return PIXELFORMAT_NONE;
}

bool VideoDevice::setInputParameters()
{
    if (!isOpen()) {
        qWarning("VideoDevice::setInputParameters: not opened");
        return false;
    }
    //### BETTER TO NOT SET ANYTHING UNTIL WE PUT IN PLACE A GOOD SOLUTION! //
#if 0
    if ((m_currentInput >= inputCount()))
        return false;
    setBrightness( getBrightness() );
    setContrast( getContrast() );
    setSaturation( getSaturation() );
    setWhiteness( getWhiteness() );
    setHue( getHue() );
#endif
    return true;
}

/*
float VideoDevice::getBrightness()
{
    if (m_currentInput < m_input.size() )
        return m_input[m_currentInput].getBrightness();
    else
        return 0;
}

float VideoDevice::setBrightness(float brightness)
{
    qDebug() << "(" << brightness << ") called.";
    m_input[m_currentInput].setBrightness(brightness); // Just to check bounds

    switch (m_driver)
    {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    case VIDEODEV_DRIVER_V4L2:
        {
            struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;

            CLEAR (queryctrl);
            queryctrl.id = V4L2_CID_BRIGHTNESS;

            if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
            {
                if (errno != EINVAL)
                {
                    qDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
                } else
                {
                    qDebug() << "Device doesn't support the Brightness control.";
                }
            } else
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                {
                qDebug() << "Brightness control is disabled.";
            } else
            {
                CLEAR (control);
                control.id = V4L2_CID_BRIGHTNESS;
                control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getBrightness());

                if (-1 == xioctl (VIDIOC_S_CTRL, &control))
                {
                    qDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
                }
            }
        }
        break;
#endif
    case VIDEODEV_DRIVER_V4L:
        {
            struct video_picture V4L_picture;
            if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                qDebug() << "VIDIOCGPICT failed (" << errno << ").";
            V4L_picture.brightness = uint(65535 * getBrightness());
            if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
                qDebug() << "Device seems to not support adjusting image brightness. Fallback to it is not yet implemented.";
        }
        break;
#endif
    case VIDEODEV_DRIVER_NONE:
    default:
        break;
    }
    return getBrightness();
}

float VideoDevice::getContrast()
{
    if (m_currentInput < m_input.size() )
        return m_input[m_currentInput].getContrast();
    else
        return 0;
}

float VideoDevice::setContrast(float contrast)
{
    qDebug() << "(" << contrast << ") called.";
    m_input[m_currentInput].setContrast(contrast); // Just to check bounds

    switch (m_driver)
    {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    case VIDEODEV_DRIVER_V4L2:
        {
            struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;

            CLEAR (queryctrl);
            queryctrl.id = V4L2_CID_CONTRAST;

            if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
            {
                if (errno != EINVAL)
                {
                    qDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
                } else
                {
                    qDebug() << "Device doesn't support the Contrast control.";
                }
            } else
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                {
                qDebug() << "Contrast control is disabled.";
            } else
            {
                CLEAR (control);
                control.id = V4L2_CID_CONTRAST;
                control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getContrast());

                if (-1 == xioctl (VIDIOC_S_CTRL, &control))
                {
                    qDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
                }
            }
        }
        break;
#endif
    case VIDEODEV_DRIVER_V4L:
        {
            struct video_picture V4L_picture;
            if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                qDebug() << "VIDIOCGPICT failed (" << errno << ").";
            V4L_picture.contrast = uint(65535*getContrast());
            if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
                qDebug() << "Device seems to not support adjusting image contrast. Fallback to it is not yet implemented.";
        }
        break;
#endif
    case VIDEODEV_DRIVER_NONE:
    default:
        break;
    }
    return getContrast();
}

float VideoDevice::getSaturation()
{
    if (m_currentInput < m_input.size() )
        return m_input[m_currentInput].getSaturation();
    else
        return 0;
}

float VideoDevice::setSaturation(float saturation)
{
    qDebug() << "(" << saturation << ") called.";
    m_input[m_currentInput].setSaturation(saturation); // Just to check bounds

    switch (m_driver)
    {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    case VIDEODEV_DRIVER_V4L2:
        {
            struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;

            CLEAR (queryctrl);
            queryctrl.id = V4L2_CID_SATURATION;

            if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
            {
                if (errno != EINVAL)
                {
                    qDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
                } else
                {
                    qDebug() << "Device doesn't support the Saturation control.";
                }
            } else
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                {
                qDebug() << "Saturation control is disabled.";
            } else
            {
                CLEAR (control);
                control.id = V4L2_CID_SATURATION;
                control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getSaturation());

                if (-1 == xioctl (VIDIOC_S_CTRL, &control))
                {
                    qDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
                }
            }
        }
        break;
#endif
    case VIDEODEV_DRIVER_V4L:
        {
            struct video_picture V4L_picture;
            if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                qDebug() << "VIDIOCGPICT failed (" << errno << ").";
            V4L_picture.colour = uint(65535*getSaturation());
            if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
                qDebug() << "Device seems to not support adjusting image saturation. Fallback to it is not yet implemented.";
        }
        break;
#endif
    case VIDEODEV_DRIVER_NONE:
    default:
        break;
    }
    return getSaturation();
}

float VideoDevice::getWhiteness()
{
    if (m_currentInput < m_input.size() )
        return m_input[m_currentInput].getWhiteness();
    else
        return 0;
}

float VideoDevice::setWhiteness(float whiteness)
{
    qDebug() << "(" << whiteness << ") called.";
    m_input[m_currentInput].setWhiteness(whiteness); // Just to check bounds

    switch (m_driver)
    {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    case VIDEODEV_DRIVER_V4L2:
        {
            struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;

            CLEAR (queryctrl);
            queryctrl.id = V4L2_CID_WHITENESS;

            if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
            {
                if (errno != EINVAL)
                {
                    qDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
                } else
                {
                    qDebug() << "Device doesn't support the Whiteness control.";
                }
            } else
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                {
                qDebug() << "Whiteness control is disabled.";
            } else
            {
                CLEAR (control);
                control.id = V4L2_CID_WHITENESS;
                control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getWhiteness());

                if (-1 == xioctl (VIDIOC_S_CTRL, &control))
                {
                    qDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
                }
            }
        }
        break;
#endif
    case VIDEODEV_DRIVER_V4L:
        {
            struct video_picture V4L_picture;
            if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                qDebug() << "VIDIOCGPICT failed (" << errno << ").";
            V4L_picture.whiteness = uint(65535*getWhiteness());
            if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
                qDebug() << "Device seems to not support adjusting white level. Fallback to it is not yet implemented.";
        }
        break;
#endif
    case VIDEODEV_DRIVER_NONE:
    default:
        break;
    }
    return getWhiteness();
}

float VideoDevice::getHue()
{
    if (m_currentInput < m_input.size() )
        return m_input[m_currentInput].getHue();
    else
        return 0;
}

float VideoDevice::setHue(float hue)
{
    qDebug() << "(" << hue << ") called.";
    m_input[m_currentInput].setHue(hue); // Just to check bounds

    switch (m_driver)
    {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    case VIDEODEV_DRIVER_V4L2:
        {
            struct v4l2_queryctrl queryctrl;
            struct v4l2_control control;

            CLEAR (queryctrl);
            queryctrl.id = V4L2_CID_HUE;

            if (-1 == xioctl (VIDIOC_QUERYCTRL, &queryctrl))
            {
                if (errno != EINVAL)
                {
                    qDebug() <<  "VIDIOC_QUERYCTRL failed (" << errno << ").";
                } else
                {
                    qDebug() << "Device doesn't support the Hue control.";
                }
            } else
                if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                {
                qDebug() << "Hue control is disabled.";
            } else
            {
                CLEAR (control);
                control.id = V4L2_CID_HUE;
                control.value = (__s32)((queryctrl.maximum - queryctrl.minimum)*getHue());

                if (-1 == xioctl (VIDIOC_S_CTRL, &control))
                {
                    qDebug() <<  "VIDIOC_S_CTRL failed (" << errno << ").";
                }
            }
        }
        break;
#endif
    case VIDEODEV_DRIVER_V4L:
        {
            struct video_picture V4L_picture;
            if(-1 == xioctl(VIDIOCGPICT, &V4L_picture))
                qDebug() << "VIDIOCGPICT failed (" << errno << ").";
            V4L_picture.hue = uint(65535*getHue());
            if(-1 == xioctl(VIDIOCSPICT,&V4L_picture))
                qDebug() << "Device seems to not support adjusting image hue. Fallback to it is not yet implemented.";
        }
        break;
#endif
    case VIDEODEV_DRIVER_NONE:
    default:
        break;
    }
    return getHue();
}
*/

bool VideoDevice::getAutoBrightnessContrast() const
{
    if (m_currentInput < m_input.size())
        return m_input[m_currentInput].getAutoBrightnessContrast();
    else
        return false;
}

bool VideoDevice::setAutoBrightnessContrast(bool brightnesscontrast)
{
    qDebug() << "VideoDevice::setAutoBrightnessContrast(" << brightnesscontrast << ") called.";
    if (m_currentInput < m_input.size()) {
        m_input[m_currentInput].setAutoBrightnessContrast(brightnesscontrast);
        return m_input[m_currentInput].getAutoBrightnessContrast();
    }
    return false;
}

bool VideoDevice::getAutoColorCorrection() const
{
    if (m_currentInput < m_input.size())
        return m_input[m_currentInput].getAutoColorCorrection();
    return false;
}

bool VideoDevice::setAutoColorCorrection(bool colorcorrection)
{
    qDebug() << "VideoDevice::setAutoColorCorrection(" << colorcorrection << ") called.";
    if (m_currentInput < m_input.size()) {
        m_input[m_currentInput].setAutoColorCorrection(colorcorrection);
        return m_input[m_currentInput].getAutoColorCorrection();
    }
    return false;
}

bool VideoDevice::getImageAsMirror()
{
    if (m_currentInput < m_input.size())
        return m_input[m_currentInput].getImageAsMirror();
    return false;
}

bool VideoDevice::setImageAsMirror(bool imageasmirror)
{
    qDebug() << "VideoDevice::setImageAsMirror(" << imageasmirror << ") called.";
    if (m_currentInput < m_input.size()) {
        m_input[m_currentInput].setImageAsMirror(imageasmirror);
        return m_input[m_currentInput].getImageAsMirror();
    }
    return false;
}

PixelFormat VideoDevice::pixelFormatFromPlatform(int platform) const
{
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            switch (platform) {
                case 0                      : return PIXELFORMAT_NONE;      break;

                // Packed RGB formats
                case V4L2_PIX_FMT_RGB332	: return PIXELFORMAT_RGB332;	break;
#if defined( V4L2_PIX_FMT_RGB444 )
                case V4L2_PIX_FMT_RGB444	: return PIXELFORMAT_RGB444;	break;
#endif
                case V4L2_PIX_FMT_RGB555	: return PIXELFORMAT_RGB555;	break;
                case V4L2_PIX_FMT_RGB565	: return PIXELFORMAT_RGB565;	break;
                case V4L2_PIX_FMT_RGB555X	: return PIXELFORMAT_RGB555X;	break;
                case V4L2_PIX_FMT_RGB565X	: return PIXELFORMAT_RGB565X;	break;
                case V4L2_PIX_FMT_BGR24		: return PIXELFORMAT_BGR24;     break;
                case V4L2_PIX_FMT_RGB24		: return PIXELFORMAT_RGB24;     break;
                case V4L2_PIX_FMT_BGR32		: return PIXELFORMAT_BGR32;     break;
                case V4L2_PIX_FMT_RGB32		: return PIXELFORMAT_RGB32;     break;

                // Bayer RGB format
                case V4L2_PIX_FMT_SBGGR8	: return PIXELFORMAT_SBGGR8;	break;

                // YUV formats
                case V4L2_PIX_FMT_GREY		: return PIXELFORMAT_GREY;      break;
                case V4L2_PIX_FMT_YUYV		: return PIXELFORMAT_YUYV;      break;
                case V4L2_PIX_FMT_UYVY		: return PIXELFORMAT_UYVY;      break;
                case V4L2_PIX_FMT_YUV420	: return PIXELFORMAT_YUV420P;	break;
                case V4L2_PIX_FMT_YUV422P	: return PIXELFORMAT_YUV422P;	break;

                // Compressed formats
                case V4L2_PIX_FMT_JPEG		: return PIXELFORMAT_JPEG;      break;
                case V4L2_PIX_FMT_MPEG		: return PIXELFORMAT_MPEG;      break;

                // Reserved formats
                case V4L2_PIX_FMT_DV		: return PIXELFORMAT_DV;        break;
                case V4L2_PIX_FMT_ET61X251	: return PIXELFORMAT_ET61X251;	break;
                case V4L2_PIX_FMT_HI240		: return PIXELFORMAT_HI240;     break;
#if defined( V4L2_PIX_FMT_HM12 )
                case V4L2_PIX_FMT_HM12		: return PIXELFORMAT_HM12;      break;
#endif
                case V4L2_PIX_FMT_MJPEG		: return PIXELFORMAT_MJPEG;     break;
                case V4L2_PIX_FMT_PWC1		: return PIXELFORMAT_PWC1;      break;
                case V4L2_PIX_FMT_PWC2		: return PIXELFORMAT_PWC2;      break;
                case V4L2_PIX_FMT_SN9C10X	: return PIXELFORMAT_SN9C10X;	break;
                case V4L2_PIX_FMT_WNVA		: return PIXELFORMAT_WNVA;      break;
                case V4L2_PIX_FMT_YYUV		: return PIXELFORMAT_YYUV;      break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_V4L:
            switch (platform) {
                case 0                      : return PIXELFORMAT_NONE;      break;
                case VIDEO_PALETTE_GREY		: return PIXELFORMAT_GREY;      break;
                case VIDEO_PALETTE_HI240	: return PIXELFORMAT_RGB332;	break;
                case VIDEO_PALETTE_RGB555	: return PIXELFORMAT_RGB555;	break;
                case VIDEO_PALETTE_RGB565	: return PIXELFORMAT_RGB565;	break;
                case VIDEO_PALETTE_RGB24	: return PIXELFORMAT_RGB24;     break;
                case VIDEO_PALETTE_RGB32	: return PIXELFORMAT_RGB32;     break;
                case VIDEO_PALETTE_YUYV		: return PIXELFORMAT_YUYV;      break;
                case VIDEO_PALETTE_UYVY		: return PIXELFORMAT_UYVY;      break;
                case VIDEO_PALETTE_YUV420	:
                case VIDEO_PALETTE_YUV420P	: return PIXELFORMAT_YUV420P;   break;
                case VIDEO_PALETTE_YUV422P	: return PIXELFORMAT_YUV422P;	break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            return PIXELFORMAT_NONE;	break;
    }
    return PIXELFORMAT_NONE;
}

int VideoDevice::pixelFormatToPlatform(PixelFormat pixelformat) const
{
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            switch (pixelformat) {
                case PIXELFORMAT_NONE	: return 0;                     break;

                // Packed RGB formats
                case PIXELFORMAT_RGB332	: return V4L2_PIX_FMT_RGB332;	break;
#if defined(V4L2_PIX_FMT_RGB444)
                case PIXELFORMAT_RGB444	: return V4L2_PIX_FMT_RGB444;	break;
#endif
                case PIXELFORMAT_RGB555	: return V4L2_PIX_FMT_RGB555;	break;
                case PIXELFORMAT_RGB565	: return V4L2_PIX_FMT_RGB565;	break;
                case PIXELFORMAT_RGB555X: return V4L2_PIX_FMT_RGB555X;	break;
                case PIXELFORMAT_RGB565X: return V4L2_PIX_FMT_RGB565X;	break;
                case PIXELFORMAT_BGR24	: return V4L2_PIX_FMT_BGR24;	break;
                case PIXELFORMAT_RGB24	: return V4L2_PIX_FMT_RGB24;	break;
                case PIXELFORMAT_BGR32	: return V4L2_PIX_FMT_BGR32;	break;
                case PIXELFORMAT_RGB32	: return V4L2_PIX_FMT_RGB32;	break;

                // Bayer RGB format
                case PIXELFORMAT_SBGGR8	: return V4L2_PIX_FMT_SBGGR8;	break;

                // YUV formats
                case PIXELFORMAT_GREY	: return V4L2_PIX_FMT_GREY;     break;
                case PIXELFORMAT_YUYV	: return V4L2_PIX_FMT_YUYV;     break;
                case PIXELFORMAT_UYVY	: return V4L2_PIX_FMT_UYVY;     break;
                case PIXELFORMAT_YUV420P: return V4L2_PIX_FMT_YUV420;	break;
                case PIXELFORMAT_YUV422P: return V4L2_PIX_FMT_YUV422P;	break;

                // Compressed formats
                case PIXELFORMAT_JPEG	: return V4L2_PIX_FMT_JPEG;     break;
                case PIXELFORMAT_MPEG	: return V4L2_PIX_FMT_MPEG;     break;

                // Reserved formats
                case PIXELFORMAT_DV     : return V4L2_PIX_FMT_DV;       break;
                case PIXELFORMAT_ET61X251:return V4L2_PIX_FMT_ET61X251; break;
                case PIXELFORMAT_HI240	: return V4L2_PIX_FMT_HI240;	break;
#if defined( V4L2_PIX_FMT_HM12 )
                case PIXELFORMAT_HM12	: return V4L2_PIX_FMT_HM12;     break;
#endif
                case PIXELFORMAT_MJPEG	: return V4L2_PIX_FMT_MJPEG;	break;
                case PIXELFORMAT_PWC1	: return V4L2_PIX_FMT_PWC1;     break;
                case PIXELFORMAT_PWC2	: return V4L2_PIX_FMT_PWC2;     break;
                case PIXELFORMAT_SN9C10X: return V4L2_PIX_FMT_SN9C10X;	break;
                case PIXELFORMAT_WNVA	: return V4L2_PIX_FMT_WNVA;     break;
                case PIXELFORMAT_YYUV	: return V4L2_PIX_FMT_YYUV;     break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_V4L:
            switch (pixelformat) {
                case PIXELFORMAT_NONE	: return 0;                     break;

                // Packed RGB formats
                case PIXELFORMAT_RGB332	: return VIDEO_PALETTE_HI240;	break;
                case PIXELFORMAT_RGB444	: return 0;                     break;
                case PIXELFORMAT_RGB555	: return VIDEO_PALETTE_RGB555;	break;
                case PIXELFORMAT_RGB565	: return VIDEO_PALETTE_RGB565;	break;
                case PIXELFORMAT_RGB555X: return 0;                     break;
                case PIXELFORMAT_RGB565X: return 0;                     break;
                case PIXELFORMAT_BGR24	: return 0;                     break;
                case PIXELFORMAT_RGB24	: return VIDEO_PALETTE_RGB24;	break;
                case PIXELFORMAT_BGR32	: return 0;                     break;
                case PIXELFORMAT_RGB32	: return VIDEO_PALETTE_RGB32;	break;

                // Bayer RGB format
                case PIXELFORMAT_SBGGR8	: return 0;                     break;

                // YUV formats
                case PIXELFORMAT_GREY	: return VIDEO_PALETTE_GREY;	break;
                case PIXELFORMAT_YUYV	: return VIDEO_PALETTE_YUYV;	break;
                case PIXELFORMAT_UYVY	: return VIDEO_PALETTE_UYVY;	break;
                case PIXELFORMAT_YUV420P: return VIDEO_PALETTE_YUV420;	break;
                case PIXELFORMAT_YUV422P: return VIDEO_PALETTE_YUV422P;	break;

                // Compressed formats
                case PIXELFORMAT_JPEG	: return 0;                     break;
                case PIXELFORMAT_MPEG	: return 0;                     break;

                // Reserved formats
                case PIXELFORMAT_DV     : return 0;                     break;
                case PIXELFORMAT_ET61X251:return 0;                 	break;
                case PIXELFORMAT_HI240	: return VIDEO_PALETTE_HI240;	break;
                case PIXELFORMAT_HM12	: return 0;                     break;
                case PIXELFORMAT_MJPEG	: return 0;                     break;
                case PIXELFORMAT_PWC1	: return 0;                     break;
                case PIXELFORMAT_PWC2	: return 0;                     break;
                case PIXELFORMAT_SN9C10X: return 0;                     break;
                case PIXELFORMAT_WNVA	: return 0;                     break;
                case PIXELFORMAT_YYUV	: return 0;                     break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            return PIXELFORMAT_NONE;	break;
    }
    return PIXELFORMAT_NONE;
}

int VideoDevice::pixelFormatDepth(PixelFormat pixelformat) const
{
    switch (pixelformat) {
        case PIXELFORMAT_NONE	: return 0;	break;

        // Packed RGB formats
        case PIXELFORMAT_RGB332	: return 8;	break;
        case PIXELFORMAT_RGB444	: return 16;break;
        case PIXELFORMAT_RGB555	: return 16;break;
        case PIXELFORMAT_RGB565	: return 16;break;
        case PIXELFORMAT_RGB555X: return 16;break;
        case PIXELFORMAT_RGB565X: return 16;break;
        case PIXELFORMAT_BGR24	: return 24;break;
        case PIXELFORMAT_RGB24	: return 24;break;
        case PIXELFORMAT_BGR32	: return 32;break;
        case PIXELFORMAT_RGB32	: return 32;break;

        // Bayer RGB format
        case PIXELFORMAT_SBGGR8	: return 0;	break;

        // YUV formats
        case PIXELFORMAT_GREY	: return 8;	break;
        case PIXELFORMAT_YUYV	: return 16;break;
        case PIXELFORMAT_UYVY	: return 16;break;
        case PIXELFORMAT_YUV420P: return 16;break;
        case PIXELFORMAT_YUV422P: return 16;break;

        // Compressed formats
        case PIXELFORMAT_JPEG	: return 0;	break;
        case PIXELFORMAT_MPEG	: return 0;	break;

        // Reserved formats
        case PIXELFORMAT_DV     : return 0;	break;
        case PIXELFORMAT_ET61X251:return 0;	break;
        case PIXELFORMAT_HI240	: return 8;	break;
        case PIXELFORMAT_HM12	: return 0;	break;
        case PIXELFORMAT_MJPEG	: return 0;	break;
        case PIXELFORMAT_PWC1	: return 0;	break;
        case PIXELFORMAT_PWC2	: return 0;	break;
        case PIXELFORMAT_SN9C10X: return 0;	break;
        case PIXELFORMAT_WNVA	: return 0;	break;
        case PIXELFORMAT_YYUV	: return 0;	break;
    }
    return 0;
}

QString VideoDevice::pixelFormatName(PixelFormat pixelformat) const
{
    QString returnvalue("None");
    switch (pixelformat) {
        case PIXELFORMAT_NONE	: returnvalue = "None";             break;

        // Packed RGB formats
        case PIXELFORMAT_RGB332	: returnvalue = "8-bit RGB332";		break;
        case PIXELFORMAT_RGB444	: returnvalue = "8-bit RGB444";		break;
        case PIXELFORMAT_RGB555	: returnvalue = "16-bit RGB555";	break;
        case PIXELFORMAT_RGB565	: returnvalue = "16-bit RGB565";	break;
        case PIXELFORMAT_RGB555X: returnvalue = "16-bit RGB555X";	break;
        case PIXELFORMAT_RGB565X: returnvalue = "16-bit RGB565X";	break;
        case PIXELFORMAT_BGR24	: returnvalue = "24-bit BGR24";		break;
        case PIXELFORMAT_RGB24	: returnvalue = "24-bit RGB24";		break;
        case PIXELFORMAT_BGR32	: returnvalue = "32-bit BGR32";		break;
        case PIXELFORMAT_RGB32	: returnvalue = "32-bit RGB32";		break;

        // Bayer RGB format
        case PIXELFORMAT_SBGGR8	: returnvalue = "Bayer RGB format";	break;

        // YUV formats
        case PIXELFORMAT_GREY	: returnvalue = "8-bit Grayscale";	break;
        case PIXELFORMAT_YUYV	: returnvalue = "Packed YUV 4:2:2";	break;
        case PIXELFORMAT_UYVY	: returnvalue = "Packed YVU 4:2:2";	break;
        case PIXELFORMAT_YUV420P: returnvalue = "Planar YUV 4:2:0";	break;
        case PIXELFORMAT_YUV422P: returnvalue = "Planar YUV 4:2:2";	break;

        // Compressed formats
        case PIXELFORMAT_JPEG	: returnvalue = "JPEG image";		break;
        case PIXELFORMAT_MPEG	: returnvalue = "MPEG stream";		break;

        // Reserved formats
        case PIXELFORMAT_DV	: returnvalue = "DV (unknown)";         break;
        case PIXELFORMAT_ET61X251:returnvalue = "ET61X251";         break;
        case PIXELFORMAT_HI240	: returnvalue = "8-bit HI240 (RGB332)";	break;
        case PIXELFORMAT_HM12	: returnvalue = "Packed YUV 4:2:2";	break;
        case PIXELFORMAT_MJPEG	: returnvalue = "8-bit Grayscale";	break;
        case PIXELFORMAT_PWC1	: returnvalue = "PWC1";             break;
        case PIXELFORMAT_PWC2	: returnvalue = "PWC2";             break;
        case PIXELFORMAT_SN9C10X: returnvalue = "SN9C102";          break;
        case PIXELFORMAT_WNVA	: returnvalue = "Winnov Videum";	break;
        case PIXELFORMAT_YYUV	: returnvalue = "YYUV (unknown)";	break;
    }
    return returnvalue;
}

QString VideoDevice::pixelFormatNamePlatform(int pixelformat) const
{
    QString returnvalue("None");
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            switch (pixelformat) {
                case 0                      : returnvalue = pixelFormatName(PIXELFORMAT_NONE);      break;

                // Packed RGB formats
                case V4L2_PIX_FMT_RGB332	: returnvalue = pixelFormatName(PIXELFORMAT_RGB332);	break;
#if defined(V4L2_PIX_FMT_RGB444)
                case V4L2_PIX_FMT_RGB444	: returnvalue = pixelFormatName(PIXELFORMAT_RGB444);	break;
#endif
                case V4L2_PIX_FMT_RGB555	: returnvalue = pixelFormatName(PIXELFORMAT_RGB555);	break;
                case V4L2_PIX_FMT_RGB565	: returnvalue = pixelFormatName(PIXELFORMAT_RGB565);	break;
                case V4L2_PIX_FMT_RGB555X	: returnvalue = pixelFormatName(PIXELFORMAT_RGB555X);	break;
                case V4L2_PIX_FMT_RGB565X	: returnvalue = pixelFormatName(PIXELFORMAT_RGB565X);	break;
                case V4L2_PIX_FMT_BGR24		: returnvalue = pixelFormatName(PIXELFORMAT_BGR24);     break;
                case V4L2_PIX_FMT_RGB24		: returnvalue = pixelFormatName(PIXELFORMAT_RGB24);     break;
                case V4L2_PIX_FMT_BGR32		: returnvalue = pixelFormatName(PIXELFORMAT_BGR32);     break;
                case V4L2_PIX_FMT_RGB32		: returnvalue = pixelFormatName(PIXELFORMAT_RGB32);     break;

                // Bayer RGB format
                case V4L2_PIX_FMT_SBGGR8	: returnvalue = pixelFormatName(PIXELFORMAT_SBGGR8);	break;

                // YUV formats
                case V4L2_PIX_FMT_GREY		: returnvalue = pixelFormatName(PIXELFORMAT_GREY);      break;
                case V4L2_PIX_FMT_YUYV		: returnvalue = pixelFormatName(PIXELFORMAT_YUYV);      break;
                case V4L2_PIX_FMT_UYVY		: returnvalue = pixelFormatName(PIXELFORMAT_UYVY);      break;
                case V4L2_PIX_FMT_YUV420	: returnvalue = pixelFormatName(PIXELFORMAT_YUV420P);	break;
                case V4L2_PIX_FMT_YUV422P	: returnvalue = pixelFormatName(PIXELFORMAT_YUV422P);	break;

                // Compressed formats
                case V4L2_PIX_FMT_JPEG		: returnvalue = pixelFormatName(PIXELFORMAT_JPEG);      break;
                case V4L2_PIX_FMT_MPEG		: returnvalue = pixelFormatName(PIXELFORMAT_MPEG);      break;

                // Reserved formats
                case V4L2_PIX_FMT_DV		: returnvalue = pixelFormatName(PIXELFORMAT_DV);        break;
                case V4L2_PIX_FMT_ET61X251	: returnvalue = pixelFormatName(PIXELFORMAT_ET61X251);	break;
                case V4L2_PIX_FMT_HI240		: returnvalue = pixelFormatName(PIXELFORMAT_HI240);     break;
#if defined( V4L2_PIX_FMT_HM12 )
                case V4L2_PIX_FMT_HM12		: returnvalue = pixelFormatName(PIXELFORMAT_HM12);      break;
#endif
                case V4L2_PIX_FMT_MJPEG		: returnvalue = pixelFormatName(PIXELFORMAT_MJPEG);     break;
                case V4L2_PIX_FMT_PWC1		: returnvalue = pixelFormatName(PIXELFORMAT_PWC1);      break;
                case V4L2_PIX_FMT_PWC2		: returnvalue = pixelFormatName(PIXELFORMAT_PWC2);      break;
                case V4L2_PIX_FMT_SN9C10X	: returnvalue = pixelFormatName(PIXELFORMAT_SN9C10X);	break;
                case V4L2_PIX_FMT_WNVA		: returnvalue = pixelFormatName(PIXELFORMAT_WNVA);      break;
                case V4L2_PIX_FMT_YYUV		: returnvalue = pixelFormatName(PIXELFORMAT_YYUV);      break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_V4L:
            switch (pixelformat) {
                case VIDEO_PALETTE_GREY		: returnvalue = pixelFormatName(PIXELFORMAT_GREY);      break;
                case VIDEO_PALETTE_HI240	: returnvalue = pixelFormatName(PIXELFORMAT_RGB332);	break;
                case VIDEO_PALETTE_RGB555	: returnvalue = pixelFormatName(PIXELFORMAT_RGB555);	break;
                case VIDEO_PALETTE_RGB565	: returnvalue = pixelFormatName(PIXELFORMAT_RGB565);	break;
                case VIDEO_PALETTE_RGB24	: returnvalue = pixelFormatName(PIXELFORMAT_RGB24);     break;
                case VIDEO_PALETTE_RGB32	: returnvalue = pixelFormatName(PIXELFORMAT_RGB32);     break;
                case VIDEO_PALETTE_YUYV		: returnvalue = pixelFormatName(PIXELFORMAT_YUYV);      break;
                case VIDEO_PALETTE_UYVY		: returnvalue = pixelFormatName(PIXELFORMAT_UYVY);      break;
                case VIDEO_PALETTE_YUV420	:
                case VIDEO_PALETTE_YUV420P	: returnvalue = pixelFormatName(PIXELFORMAT_YUV420P);	break;
                case VIDEO_PALETTE_YUV422P	: returnvalue = pixelFormatName(PIXELFORMAT_YUV422P);	break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }
    return returnvalue;
}

/*
__u64 VideoDevice::signalStandardCode(signal_standard standard)
{
    switch (m_driver)
    {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
    case VIDEODEV_DRIVER_V4L2:
        switch (standard)
        {
        case STANDARD_PAL_B	: return V4L2_STD_PAL_B;	break;
        case STANDARD_PAL_B1	: return V4L2_STD_PAL_B1;	break;
        case STANDARD_PAL_G	: return V4L2_STD_PAL_G;	break;
        case STANDARD_PAL_H	: return V4L2_STD_PAL_H;	break;
        case STANDARD_PAL_I	: return V4L2_STD_PAL_I;	break;
        case STANDARD_PAL_D	: return V4L2_STD_PAL_D;	break;
        case STANDARD_PAL_D1	: return V4L2_STD_PAL_D1;	break;
        case STANDARD_PAL_K	: return V4L2_STD_PAL_K;	break;

        case STANDARD_PAL_M	: return V4L2_STD_PAL_M;	break;
        case STANDARD_PAL_N	: return V4L2_STD_PAL_N;	break;
        case STANDARD_PAL_Nc	: return V4L2_STD_PAL_Nc;	break;
        case STANDARD_PAL_60	: return V4L2_STD_PAL_60;	break;

        case STANDARD_NTSC_M	: return V4L2_STD_NTSC_M;	break;
        case STANDARD_NTSC_M_JP	: return V4L2_STD_NTSC_M_JP;	break;
        case STANDARD_NTSC_443	: return V4L2_STD_NTSC_443;	break;
        case STANDARD_NTSC_M_KR	: return V4L2_STD_NTSC_M_KR;	break;

        case STANDARD_SECAM_B	: return V4L2_STD_SECAM_B;	break;
        case STANDARD_SECAM_D	: return V4L2_STD_SECAM_D;	break;
        case STANDARD_SECAM_G	: return V4L2_STD_SECAM_G;	break;
        case STANDARD_SECAM_H	: return V4L2_STD_SECAM_H;	break;
        case STANDARD_SECAM_K	: return V4L2_STD_SECAM_K;	break;
        case STANDARD_SECAM_K1	: return V4L2_STD_SECAM_K1;	break;
        case STANDARD_SECAM_L	: return V4L2_STD_SECAM_L;	break;
        case STANDARD_SECAM_LC	: return V4L2_STD_SECAM_LC;	break;

        case STANDARD_ATSC_8_VSB: return V4L2_STD_ATSC_8_VSB;	break;
        case STANDARD_ATSC_16_VSB:return V4L2_STD_ATSC_16_VSB;	break;

        case STANDARD_PAL_BG	: return V4L2_STD_PAL_BG;	break;
        case STANDARD_PAL_DK	: return V4L2_STD_PAL_DK;	break;
        case STANDARD_PAL	: return V4L2_STD_PAL;		break;
        case STANDARD_NTSC	: return V4L2_STD_NTSC;		break;
        case STANDARD_SECAM_DK	: return V4L2_STD_SECAM_DK;	break;
        case STANDARD_SECAM	: return V4L2_STD_SECAM;	break;

        case STANDARD_MN	: return V4L2_STD_MN;		break;
        case STANDARD_B		: return V4L2_STD_B;		break;
        case STANDARD_GH	: return V4L2_STD_GH;		break;
        case STANDARD_DK	: return V4L2_STD_DK;		break;

        case STANDARD_525_60	: return V4L2_STD_525_60;	break;
        case STANDARD_625_50	: return V4L2_STD_625_50;	break;
        case STANDARD_ATSC	: return V4L2_STD_ATSC;		break;

        case STANDARD_UNKNOWN	: return V4L2_STD_UNKNOWN;	break;
        case STANDARD_ALL	: return V4L2_STD_ALL;		break;
        }
        break;
#endif
    case VIDEODEV_DRIVER_V4L:
        switch (standard)
        {
        case STANDARD_PAL_B	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_B1	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_G	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_H	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_I	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_D	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_D1	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_K	: return VIDEO_MODE_PAL;	break;

        case STANDARD_PAL_M	: return 5;	break;	// Undocumented value found to be compatible with V4L bttv driver
        case STANDARD_PAL_N	: return 6;	break;	// Undocumented value found to be compatible with V4L bttv driver
        case STANDARD_PAL_Nc	: return 4;	break;	// Undocumented value found to be compatible with V4L bttv driver
        case STANDARD_PAL_60	: return VIDEO_MODE_PAL;	break;

        case STANDARD_NTSC_M	: return VIDEO_MODE_NTSC;	break;
        case STANDARD_NTSC_M_JP	: return 7;	break;	// Undocumented value found to be compatible with V4L bttv driver
        case STANDARD_NTSC_443	: return VIDEO_MODE_NTSC;	break;
        case STANDARD_NTSC_M_KR	: return VIDEO_MODE_NTSC;	break;

        case STANDARD_SECAM_B	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_D	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_G	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_H	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_K	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_K1	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_L	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM_LC	: return VIDEO_MODE_SECAM;	break;

        case STANDARD_ATSC_8_VSB: return VIDEO_MODE_AUTO;	break;
        case STANDARD_ATSC_16_VSB:return VIDEO_MODE_AUTO;	break;

        case STANDARD_PAL_BG	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL_DK	: return VIDEO_MODE_PAL;	break;
        case STANDARD_PAL	: return VIDEO_MODE_PAL;	break;
        case STANDARD_NTSC	: return VIDEO_MODE_NTSC;	break;
        case STANDARD_SECAM_DK	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_SECAM	: return VIDEO_MODE_SECAM;	break;

        case STANDARD_MN	: return VIDEO_MODE_AUTO;	break;
        case STANDARD_B		: return VIDEO_MODE_AUTO;	break;
        case STANDARD_GH	: return VIDEO_MODE_AUTO;	break;
        case STANDARD_DK	: return VIDEO_MODE_AUTO;	break;

        case STANDARD_525_60	: return VIDEO_MODE_PAL;	break;
        case STANDARD_625_50	: return VIDEO_MODE_SECAM;	break;
        case STANDARD_ATSC	: return VIDEO_MODE_AUTO;	break;

        case STANDARD_UNKNOWN	: return VIDEO_MODE_AUTO;	break;
        case STANDARD_ALL	: return VIDEO_MODE_AUTO;	break;
        }
        break;
#endif
    case VIDEODEV_DRIVER_NONE:
    default:
        return STANDARD_UNKNOWN;	break;
    }
    return STANDARD_UNKNOWN;
}

QString VideoDevice::signalStandardName(signal_standard standard) const
{
    QString returnvalue;
    switch (standard) {
        case STANDARD_PAL_B     : returnvalue = "PAL-B";        break;
        case STANDARD_PAL_B1	: returnvalue = "PAL-B1";       break;
        case STANDARD_PAL_G     : returnvalue = "PAL-G";        break;
        case STANDARD_PAL_H     : returnvalue = "PAL-H";        break;
        case STANDARD_PAL_I     : returnvalue = "PAL-I";        break;
        case STANDARD_PAL_D     : returnvalue = "PAL-D";        break;
        case STANDARD_PAL_D1	: returnvalue = "PAL-D1";       break;
        case STANDARD_PAL_K     : returnvalue = "PAL-K";        break;

        case STANDARD_PAL_M     : returnvalue = "PAL-M";        break;
        case STANDARD_PAL_N     : returnvalue = "PAL-N";        break;
        case STANDARD_PAL_Nc	: returnvalue = "PAL-Nc";       break;
        case STANDARD_PAL_60	: returnvalue = "PAL-60";       break;

        case STANDARD_NTSC_M	: returnvalue = "NTSC-M";       break;
        case STANDARD_NTSC_M_JP	: returnvalue = "NTSC-M(JP)";	break;
        case STANDARD_NTSC_443	: returnvalue = "NTSC-443";     break;
        case STANDARD_NTSC_M_KR	: returnvalue = "NTSC-M(KR)";	break;

        case STANDARD_SECAM_B	: returnvalue = "SECAM-B";      break;
        case STANDARD_SECAM_D	: returnvalue = "SECAM-D";      break;
        case STANDARD_SECAM_G	: returnvalue = "SECAM-G";      break;
        case STANDARD_SECAM_H	: returnvalue = "SECAM-H";      break;
        case STANDARD_SECAM_K	: returnvalue = "SECAM-K";      break;
        case STANDARD_SECAM_K1	: returnvalue = "SECAM-K1";     break;
        case STANDARD_SECAM_L	: returnvalue = "SECAM-L";      break;
        case STANDARD_SECAM_LC	: returnvalue = "SECAM-LC";     break;

        case STANDARD_ATSC_8_VSB: returnvalue = "ATSC-8-VSB";	break;
        case STANDARD_ATSC_16_VSB:returnvalue = "ATSC-16-VSB";	break;

        case STANDARD_PAL_BG	: returnvalue = "PAL-BG";       break;
        case STANDARD_PAL_DK	: returnvalue = "PAL-DK";       break;
        case STANDARD_PAL       : returnvalue = "PAL";          break;
        case STANDARD_NTSC      : returnvalue = "NTSC";         break;
        case STANDARD_SECAM_DK  : returnvalue = "SECAM-DK";     break;
        case STANDARD_SECAM     : returnvalue = "SECAM";        break;

        case STANDARD_MN        : returnvalue = "DK";           break;
        case STANDARD_B         : returnvalue = "B";            break;
        case STANDARD_GH        : returnvalue = "GH";           break;
        case STANDARD_DK        : returnvalue = "DK";           break;

        case STANDARD_525_60	: returnvalue = "525 lines 60Hz";break;
        case STANDARD_625_50	: returnvalue = "625 lines 50Hz";break;
        case STANDARD_ATSC      : returnvalue = "ATSC";         break;

        case STANDARD_UNKNOWN	: returnvalue = "Unknown";      break;
        case STANDARD_ALL       : returnvalue = "All";          break;
        default                 : returnvalue = "Really Unknown";break;
    }
    return returnvalue;
}

QString VideoDevice::signalStandardName(int standard) const
{
    QString returnvalue = "None";
    switch (m_driver) {
#ifdef Q_OS_LINUX
#ifdef V4L2_CAP_VIDEO_CAPTURE
        case VIDEODEV_DRIVER_V4L2:
            switch (standard)
            {
                case V4L2_STD_PAL_B     : returnvalue = signalStandardName(STANDARD_PAL_B);     break;
                case V4L2_STD_PAL_B1	: returnvalue = signalStandardName(STANDARD_PAL_B1);   	break;
                case V4L2_STD_PAL_G     : returnvalue = signalStandardName(STANDARD_PAL_G);     break;
                case V4L2_STD_PAL_H     : returnvalue = signalStandardName(STANDARD_PAL_H);     break;
                case V4L2_STD_PAL_I     : returnvalue = signalStandardName(STANDARD_PAL_I);     break;
                case V4L2_STD_PAL_D     : returnvalue = signalStandardName(STANDARD_PAL_D);     break;
                case V4L2_STD_PAL_D1	: returnvalue = signalStandardName(STANDARD_PAL_D1);	break;
                case V4L2_STD_PAL_K     : returnvalue = signalStandardName(STANDARD_PAL_K);     break;
                case V4L2_STD_PAL_M     : returnvalue = signalStandardName(STANDARD_PAL_M);     break;
                case V4L2_STD_PAL_N     : returnvalue = signalStandardName(STANDARD_PAL_N);     break;
                case V4L2_STD_PAL_Nc	: returnvalue = signalStandardName(STANDARD_PAL_Nc);	break;
                case V4L2_STD_PAL_60	: returnvalue = signalStandardName(STANDARD_PAL_60);	break;
                case V4L2_STD_NTSC_M	: returnvalue = signalStandardName(STANDARD_NTSC_M);	break;
                case V4L2_STD_NTSC_M_JP	: returnvalue = signalStandardName(STANDARD_NTSC_M_JP);	break;
                case V4L2_STD_NTSC_443	: returnvalue = signalStandardName(STANDARD_NTSC_443);	break; // Commented out because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
                case V4L2_STD_NTSC_M_KR	: returnvalue = signalStandardName(STANDARD_NTSC_M_KR);	break; // Commented out because my videodev2.h header seems to not include this standard in struct __u64 v4l2_std_id
                case V4L2_STD_SECAM_B	: returnvalue = signalStandardName(STANDARD_SECAM_B);	break;
                case V4L2_STD_SECAM_D	: returnvalue = signalStandardName(STANDARD_SECAM_D);	break;
                case V4L2_STD_SECAM_G	: returnvalue = signalStandardName(STANDARD_SECAM_G);	break;
                case V4L2_STD_SECAM_H	: returnvalue = signalStandardName(STANDARD_SECAM_H);	break;
                case V4L2_STD_SECAM_K	: returnvalue = signalStandardName(STANDARD_SECAM_K);	break;
                case V4L2_STD_SECAM_K1	: returnvalue = signalStandardName(STANDARD_SECAM_K1);	break;
                case V4L2_STD_SECAM_L	: returnvalue = signalStandardName(STANDARD_SECAM_L);	break;
                case V4L2_STD_SECAM_LC	: returnvalue = signalStandardName(STANDARD_SECAM_LC);	break;

                case V4L2_STD_ATSC_8_VSB: returnvalue = signalStandardName(STANDARD_ATSC_8_VSB);break;
                case V4L2_STD_ATSC_16_VSB:returnvalue = signalStandardName(STANDARD_ATSC_16_VSB);break;

                case V4L2_STD_PAL_BG	: returnvalue = signalStandardName(STANDARD_PAL_BG);	break;
                case V4L2_STD_PAL_DK	: returnvalue = signalStandardName(STANDARD_PAL_DK);	break;
                case V4L2_STD_PAL       : returnvalue = signalStandardName(STANDARD_PAL);       break;
                case V4L2_STD_NTSC      : returnvalue = signalStandardName(STANDARD_NTSC);      break;
                case V4L2_STD_SECAM_DK	: returnvalue = signalStandardName(STANDARD_SECAM_DK);	break;
                case V4L2_STD_SECAM     : returnvalue = signalStandardName(STANDARD_SECAM);     break;

                case V4L2_STD_MN        : returnvalue = signalStandardName(STANDARD_MN);        break;
                case V4L2_STD_B         : returnvalue = signalStandardName(STANDARD_B);         break;
                case V4L2_STD_GH        : returnvalue = signalStandardName(STANDARD_GH);        break;
                case V4L2_STD_DK        : returnvalue = signalStandardName(STANDARD_DK);        break;

                case V4L2_STD_525_60	: returnvalue = signalStandardName(STANDARD_525_60);	break;
                case V4L2_STD_625_50	: returnvalue = signalStandardName(STANDARD_625_50);	break;
                case V4L2_STD_ATSC      : returnvalue = signalStandardName(STANDARD_ATSC);      break;

                case V4L2_STD_UNKNOWN	: returnvalue = signalStandardName(STANDARD_UNKNOWN);	break;
                case V4L2_STD_ALL       : returnvalue = signalStandardName(STANDARD_ALL);       break;
            }
            break;
#endif
        case VIDEODEV_DRIVER_V4L:
            switch (standard) {
                case VIDEO_MODE_PAL     : returnvalue = signalStandardName(STANDARD_PAL);	break;
                case VIDEO_MODE_NTSC	: returnvalue = signalStandardName(STANDARD_NTSC);	break;
                case VIDEO_MODE_SECAM	: returnvalue = signalStandardName(STANDARD_SECAM);	break;
                case VIDEO_MODE_AUTO	: returnvalue = signalStandardName(STANDARD_ALL);	break;	// It must be disabled until I find a correct way to handle those non-standard bttv modes
                    //				case VIDEO_MODE_PAL_Nc	: returnvalue = signalStandardName(STANDARD_PAL_Nc);	break;	// Undocumented value found to be compatible with V4L bttv driver
                case VIDEO_MODE_PAL_M	: returnvalue = signalStandardName(STANDARD_PAL_M);	break;	// Undocumented value found to be compatible with V4L bttv driver
                case VIDEO_MODE_PAL_N	: returnvalue = signalStandardName(STANDARD_PAL_N);	break;	// Undocumented value found to be compatible with V4L bttv driver
                case VIDEO_MODE_NTSC_JP	: returnvalue = signalStandardName(STANDARD_NTSC_M_JP);	break;	// Undocumented value found to be compatible with V4L bttv driver
            }
            break;
#endif
        case VIDEODEV_DRIVER_NONE:
        default:
            break;
    }
    return returnvalue;
}
*/
bool VideoDevice::canCapture() const
{
    return m_videocapture;
}

bool VideoDevice::canChromakey() const
{
    return m_videochromakey;
}

bool VideoDevice::canScale() const
{
    return m_videoscale;
}

bool VideoDevice::canOverlay() const
{
    return m_videooverlay;
}

bool VideoDevice::canRead() const
{
    return m_videoread;
}

bool VideoDevice::canAsyncIO() const
{
    return m_videoasyncio;
}

bool VideoDevice::canStream() const
{
    return m_videostream;
}

int VideoDevice::xioctl(int request, void *arg) const
{
    int r;
    //qWarning() << "ioctl:" << m_videoFileDescriptor << request;
    do { r = ioctl(m_videoFileDescriptor, request, arg); }
    while (r == -1 && errno == EINTR);
    return r;
}

bool VideoDevice::initIoRead()
{
    if (!isOpen()) {
        qDebug("VideoDevice::initIoRead: not opened");
        return false;
    }

    // don't allocate anything: data will be copied straight to the ImageBuffer
    m_dataBuffers.clear();
    return true;
}

bool VideoDevice::initIoMmap()
{
    if (!isOpen()) {
        qDebug("VideoDevice::initIoMmap: not opened");
        return false;
    }

#ifdef V4L2_CAP_VIDEO_CAPTURE
    // request some MMAP buffers
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count  = IO_DATA_BUFFERS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(VIDIOC_REQBUFS, &req)) {
        if (errno == EINVAL) {
            qDebug() << "VideoDevice::initIoMmap:" << m_videoFileName << "does not support memory mapping";
            return false;
        } else {
            perror("VIDIOC_REQBUFS (mmap)");
            return false;
        }
    }
    if (req.count < IO_DATA_BUFFERS) {
        qDebug() << "VideoDevice::initIoMmap: insufficient buffer memory on " << m_videoFileName;
        return false;
    }

    // for each requested buffer
    m_dataBuffers.clear();
    for (unsigned int i = 0; i < req.count; ++i) {
        // configure buffer for mmap
        struct v4l2_buffer vBuffer;
        CLEAR(vBuffer);
        vBuffer.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vBuffer.memory = V4L2_MEMORY_MMAP;
        vBuffer.index  = i;
        if (-1 == xioctl(VIDIOC_QUERYBUF, &vBuffer)) {
            perror("VIDIOC_QUERYBUF (mmap)");
            return false;
        }

        // do mmap
        void * start = mmap(NULL /* start anywhere */, vBuffer.length, PROT_READ | PROT_WRITE /* required */, MAP_SHARED /* recommended */, m_videoFileDescriptor, vBuffer.m.offset);
        if (start == MAP_FAILED) {
            perror("mmap (mmap)");
            return false;
        }

        // create the DataBuffer companion
        DataBuffer d;
        d.length = vBuffer.length;
        d.start = (uchar *)start;
        m_dataBuffers.append(d);
    }
#endif

    // Makes the imagesize.data buffer size equal to the rawbuffer size
    if (m_imageBuffer.data.size() != (int)m_dataBuffers[0].length) {
        qWarning("VideoDevice::initIoMmap: image buffer size %d was different than buffer's one %d. adapted.", m_imageBuffer.data.size(), m_dataBuffers[0].length);
        m_imageBuffer.data.resize(m_dataBuffers[0].length);
    }
    qDebug() << "VideoDevice::initIoMmap: image buffer size" << m_imageBuffer.data.size();
    return true;
}

bool VideoDevice::initIoUserptr()
{
    if (!isOpen()) {
        qDebug("VideoDevice::initIoUserptr: not opened");
        return false;
    }

#ifdef V4L2_CAP_VIDEO_CAPTURE
    // request some USERPTR buffers
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count  = IO_DATA_BUFFERS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;
    if (-1 == xioctl(VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            qDebug() << "VideoDevice::initIoUserptr:" << m_videoFileName << "does not support memory mapping";
            return false;
        } else {
            perror("VIDIOC_REQBUFS (uptr)");
            return false;
        }
    }
    if (req.count < IO_DATA_BUFFERS) {
        qDebug() << "VideoDevice::initIoMmap: insufficient buffer memory on " << m_videoFileName;
        return false;
    }

    // for each requested buffer
    m_dataBuffers.clear();
    for (unsigned int i = 0; i < req.count; ++i) {
        // create the DataBuffer with userspace memory allocation
        DataBuffer d;
        d.length = m_imageBuffer.data.size();
        d.start = (uchar *)malloc(d.length);
        if (!d.start) {
            qDebug("VideoDevice::initIoUserptr: Out of memory");
            return false;
        }
        m_dataBuffers.append(d);
    }
#endif
    return true;
}

} // namespace VideoCapture
