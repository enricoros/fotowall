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

#include "VideoDevice.h"

#include "bayer.h"
#include "sonix_compress.h"

#include <QDirIterator>
#include <QDebug>
#include <QFile>
#include <QImage>

#if defined(VD_BUILD_LINUX_V4L2)

#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#define __STRICT_ANSI__
#include <cstdlib>
#include <cerrno>
#include <cstring>
// the number of buffer MMAPed or UPTRed
#define IO_DATA_BUFFERS 2

#elif defined(VD_BUILD_WIN_VFW)

#include <QLibrary>
#include <QWidget>
#include <Vfw.h>
// VFW entry points
typedef HWND (VFWAPI *PtrCapCreateCaptureWindowA) (
        LPCSTR lpszWindowName,
        __in DWORD dwStyle,
        __in int x, __in int y, __in int nWidth, __in int nHeight,
        __in_opt HWND hwndParent, __in int nID);
typedef BOOL (VFWAPI *PtrCapGetDriverDescriptionA) (
        UINT wDriverIndex,
        __out_ecount(cbName) LPSTR lpszName, __in int cbName,
        __out_ecount(cbVer) LPSTR lpszVer, __in int cbVer);
static PtrCapCreateCaptureWindowA pCapCreateCaptureWindowA = 0;
static PtrCapGetDriverDescriptionA pCapGetDriverDescriptionA = 0;
static bool vfwResolveSymbols()
{
    if (!pCapCreateCaptureWindowA) {
        QLibrary lAvicap(QString::fromAscii("avicap32"));
        pCapCreateCaptureWindowA = (PtrCapCreateCaptureWindowA)lAvicap.resolve("capCreateCaptureWindowA");
        pCapGetDriverDescriptionA = (PtrCapGetDriverDescriptionA)lAvicap.resolve("capGetDriverDescriptionA");
    }
    return pCapCreateCaptureWindowA && pCapGetDriverDescriptionA;
}
#define WINTODO qWarning("%s:%d: %s [WIN-TODO]", __FILE__, __LINE__, __FUNCTION__)

#else

#define NEWTODO qWarning("%s:%d: %s [NEW-TODO]", __FILE__, __LINE__, __FUNCTION__)

#endif

#define CLEAR(x) memset(&(x), 0, sizeof (x))

namespace VideoCapture {

VideoDevice::VideoDevice(const DeviceInfo & info)
  : m_info(info)
  , m_frameOperations(FO_None)
  , m_attributes(DA_None)
  , m_minSize(-1, -1)
  , m_maxSize(-1, -1)
  , m_currentInput(-1)
  , m_capturing(false)
#if defined(VD_BUILD_LINUX_V4L2)
  , m_videoFileDescriptor(-1)
  , m_linuxDriver(LINUX_DRIVER_NONE)
  , m_linuxIO(IO_METHOD_NONE)
#elif defined(VD_BUILD_WIN_VFW)
  , m_vWidget(0)
  , m_vHwnd(0)
#endif
{
}

VideoDevice::~VideoDevice()
{
    close();
}

QList<DeviceInfo> VideoDevice::scanDevices()
{
    QList<DeviceInfo> devices;
    int idx = 0;
#if defined(VD_BUILD_LINUX_V4L2)
    QDirIterator dirIt("/dev", QStringList() << "video*", QDir::Files | QDir::System);
    while (dirIt.hasNext()) {
        DeviceInfo info;
         info.prettyName = QString(QObject::tr("Camera %1")).arg(idx + 1);
         info.version = QString();
         info.filePath = dirIt.next();
         info.index = idx++;
        devices.append(info);
    }
#elif defined(VD_BUILD_WIN_VFW)
    if (vfwResolveSymbols()) {
        char szDeviceName[80];      // driver name
        char szDeviceVersion[80];   // driver version
        for (int i = 0; i < 10; ++i) {
            if (pCapGetDriverDescriptionA(i, (LPSTR)szDeviceName, sizeof(szDeviceName), (LPSTR)szDeviceVersion, sizeof(szDeviceVersion))) {
                DeviceInfo info;
                 info.prettyName = QString(szDeviceName);
                 info.version = QString(szDeviceVersion);
                 info.filePath = QString();
                 info.index = idx++;
                devices.append(info);
                qWarning() << "VFW devices:" << i << info.prettyName << info.version << info.filePath << info.index;
            }
        }
    }
#else
    Q_UNUSED(idx)
    qWarning("VideoDevice::devices: not implemented for this platform");
#endif
    return devices;
}

bool VideoDevice::open()
{
#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor != -1) {
        qDebug() << "VideoDevice::open: Device is already open";
        return true;
    }

    // open video device and check opening
    m_videoFileDescriptor = ::open(QFile::encodeName(m_info.filePath).constData(), O_RDWR, 0);
    if (m_videoFileDescriptor == -1) {
        qDebug() << "VideoDevice::open: Unable to open file" << m_info.filePath << "Err:"<< errno;
        return false;
    }
#elif defined(VD_BUILD_WIN_VFW)
    WINTODO;
#endif

    // check device (capabilities and stuff)
    if (!queryDeviceProperties()) {
        qDebug() << "VideoDevice::open: File" << m_info.filePath << "could not be opened.";
        close();
        return false;
    }

    // select the first input
    setCurrentInput(0);
    return true;
}

void VideoDevice::close()
{
    if (m_capturing)
        stopCapturing();
#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor != -1) {
        if (-1 == ::close(m_videoFileDescriptor))
            perror("close");
    }
    m_videoFileDescriptor = -1;
#elif defined(VD_BUILD_WIN_VFW)
    WINTODO;
#endif
}

bool VideoDevice::testAttributes(quint32 attributes) const
{
    return (m_attributes & attributes) == attributes;
}

void VideoDevice::printDeviceProperties() const
{
    qDebug() << "Device model: " << m_info.prettyName << " capabilities:";
    if (m_attributes & DA_Capture)
        qDebug() << "    Video capture";
    if (m_attributes & DA_IORead)
        qDebug() << "        Read";
    if (m_attributes & DA_IOAsync)
        qDebug() << "        Asynchronous input/output";
    if (m_attributes & DA_IOStream)
        qDebug() << "        Streaming";
    if (m_attributes & DA_ChromaKey)
        qDebug() << "    Video chromakey";
    if (m_attributes & DA_Scale)
        qDebug() << "    Video scales";
    if (m_attributes & DA_Overlay)
        qDebug() << "    Video overlay";
    qDebug() << "    Max res: " << m_maxSize;
    qDebug() << "    Min res: " << m_minSize;
    qDebug() << "    Inputs : " << inputCount();
    for (int i = 0; i < inputCount(); ++i)
        qDebug() << "      Input" << i << ": " << m_inputs[i].name << " (tuner: " << m_inputs[i].hastuner << ")";
}

int VideoDevice::inputCount() const
{
#if defined(VD_BUILD_LINUX_V4L2)
    return m_inputs.size();
#elif defined(VD_BUILD_WIN_VFW)
    return 1;
#else
    NEWTODO;
    return 0;
#endif
}

QSize VideoDevice::minSize() const
{
    return m_minSize;
}

QSize VideoDevice::maxSize() const
{
    return m_maxSize;
}

int VideoDevice::currentInput() const
{
#if defined(VD_BUILD_LINUX_V4L2)
    return m_videoFileDescriptor != -1 ? m_currentInput : 0;
#elif defined(VD_BUILD_WIN_VFW)
    return 0;
#else
    NEWTODO;
    return 0;
#endif
}

bool VideoDevice::setCurrentInput(int index)
{
    if (index < 0 || index >= inputCount())
        return false;
    if (index == m_currentInput)
        return true;

#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor == -1) {
        qWarning("VideoDevice::selectInput: device not opened");
        return false;
    }

    // select the input
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
            if (-1 == ioctl(m_videoFileDescriptor, VIDIOC_S_INPUT, &index)) {
                perror ("VIDIOC_S_INPUT");
                return false;
            }
            break;
    }
#elif defined(VD_BUILD_WIN_VFW)
    WINTODO;
#endif

    m_currentInput = index;
    qDebug() << "VideoDevice::selectInput: selected input" << index << m_inputs[index].name;

    // apply parameters to the input
    setInputParameters();
    return true;
}

QSize VideoDevice::captureSize() const
{
    return m_imageBuffer.size;
}

bool VideoDevice::setCaptureSize(const QSize & newSize)
{
    if (m_capturing) {
        qWarning("VideoDevice::setCaptureSize: can't chance parameters on-the-fly while capturing");
        return false;
    }
#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor == -1) {
        qWarning("VideoDevice::setCaptureSize: device not opened");
        return false;
    }
#endif

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
    m_imageBuffer.size = QSize( qBound(m_minSize.width(),  newSize.width(),  m_maxSize.width()  ),
                                qBound(m_minSize.height(), newSize.height(), m_maxSize.height() ) );

    // 3. change resolution for the video device
#if defined(VD_BUILD_LINUX_V4L2)
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
            struct v4l2_format format;
            CLEAR(format);
            format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(VIDIOC_G_FMT, &format))
                qDebug() << "VideoDevice::setCaptureSize: VIDIOC_G_FMT failed (" << errno << "). Returned width: " << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << " " << format.fmt.pix.width << "x" << format.fmt.pix.height;
            format.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            format.fmt.pix.width       = m_imageBuffer.size.width();
            format.fmt.pix.height      = m_imageBuffer.size.height();
            format.fmt.pix.field       = V4L2_FIELD_ANY;
            if (-1 != xioctl(VIDIOC_S_FMT, &format)) {
                // Note VIDIOC_S_FMT may change width and height.
                m_imageBuffer.size = QSize(format.fmt.pix.width, format.fmt.pix.height);
                qDebug() << "VideoDevice::setCaptureSize: using" << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << "pixel format, size:" << m_imageBuffer.size;
            } else
                qDebug() << "VideoDevice::setCaptureSize: VIDIOC_S_FMT failed (" << errno << "). Pixel format:" << pixelFormatNamePlatform(format.fmt.pix.pixelformat) << "size:" << format.fmt.pix.width << "x" << format.fmt.pix.height << "(" << newSize.width() << "x" << newSize.height() << ")";
            break;
    }
#elif defined(VD_BUILD_WIN_VFW)
    if (vfwResolveSymbols()) {

        // create a hidden top-level window for a native handle
        if (!m_vWidget) {
            m_vWidget = new QWidget;
            m_vWidget->setAttribute(Qt::WA_NativeWindow);
            m_vWidget->hide();
        }
        HWND parentHwnd = WindowFromDC(m_vWidget->getDC());
        UpdateWindow(parentHwnd);

        // start the capture preview window
        m_vHwnd = pCapCreateCaptureWindowA((LPCSTR)"Capture Window", WS_CHILD | WS_VISIBLE,
                                           0, 0, 640, 480, (HWND)parentHwnd, m_info.index);
        if (!m_vHwnd) {
            qDebug("VideoDevice::setCaptureSize: can't create capture window");
            delete m_vWidget;
            m_vWidget = 0;
            return false;
        }

        if (!capDriverConnect(m_vHwnd, m_info.index)) {
            qDebug("VideoDevice::setCaptureSize: can't connect cap driver (hwnd %x, index %d). stopping.", m_vHwnd, m_info.index);
            DestroyWindow(m_vHwnd);
            m_vHwnd = 0;
            delete m_vWidget;
            m_vWidget = 0;
            return false;
        }

        // query the real size
        CAPSTATUS capStatus;
        capGetStatus(m_vHwnd, &capStatus, sizeof(CAPSTATUS));
        m_maxSize = QSize(capStatus.uiImageWidth, capStatus.uiImageHeight);
        m_minSize = m_maxSize;
        m_imageBuffer.size = m_maxSize;

        // show format dialog?
        capDlgVideoFormat(m_vHwnd);

        // force video format?
        //capSetVideoFormat()
        //capDlgVideoSource(m_vHwnd);
        //capGetVideoFormatSize(m_vHwnd);
        //capDlgVideoDisplay(m_vHwnd);

        BITMAPINFO bi;
        if (!capGetVideoFormat(m_vHwnd, &bi, sizeof(BITMAPINFO))) {
            qDebug("VideoDevice::setCaptureSize: can't get video format. stopping.");
            return false;
        }
        if (!bi.bmiHeader.biCompression)
            pixelFormat = PIXELFORMAT_RGB24;
        else if (bi.bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
            pixelFormat = PIXELFORMAT_YUYV;
        else
            qDebug("VideoDevice::setCaptureSize: unknown pixel format (%x). trying to continue.", bi.bmiHeader.biCompression);
    }
#endif

    // 4. setup imagebuffer
    int bitPerPixel = pixelFormatDepth(pixelFormat);
    // simple hack to try to support unknown formats
    if (bitPerPixel < 1) {
        qDebug("VideoDevice::setCaptureSize: invalid depth %d, trying to assume 32", bitPerPixel);
        bitPerPixel = 32;
    }
    int bufferSize = m_imageBuffer.size.width() * m_imageBuffer.size.height() * bitPerPixel / 8;
    if (bufferSize <= 0) {
        qDebug("VideoDevice::setCaptureSize: invalid buffer size %d", bufferSize);
        return false;
    }
    //qDebug() << "VideoDevice::setCaptureSize: buffer size:" << bufferSize << bitPerPixel;
    m_imageBuffer.pixelformat = pixelFormat;
    m_imageBuffer.data.resize(bufferSize);
    return true;
}

#if defined(VD_BUILD_WIN_VFW)
static ImageBuffer * s_imageBuffer = 0;
static LRESULT videoCallback(__in HWND /*hWnd*/, __in LPVIDEOHDR lpVHdr)
{
    if (!s_imageBuffer) {
        qDebug("VFW::videoCallback: imagebuffer not ready");
        return 0;
    }
    /*if (!(lpVHdr->dwFlags & VHDR_KEYFRAME)) {
        qDebug("VFW::videoCallback: bad frame flags");
        return 0;
    }*/
    if ((int)lpVHdr->dwBufferLength < s_imageBuffer->data.size()) {
        qDebug("VFW::videoCallback: mismatch in frame buffer sizes: %d vs %d", lpVHdr->dwBufferLength, s_imageBuffer->data.size());
        return 0;
    }
    memcpy(s_imageBuffer->data.data(), lpVHdr->lpData, s_imageBuffer->data.size());
    s_imageBuffer = 0;
    return 0;
}
#endif

bool VideoDevice::startCapturing()
{
    if (m_capturing) {
        qDebug("VideoDevice::startCapturing: already started");
        return true;
    }
    if (!m_imageBuffer.size.isValid() || m_imageBuffer.data.size() < 64) {
        qDebug("VideoDevice::startCapturing: set capture size before start capturing");
        return false;
    }
#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor == -1) {
        qDebug("VideoDevice::startCapturing: not opened");
        return false;
    }
    switch (m_linuxIO) {
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

            // queue buffers
            for (int i = 0; i < m_frameMemory.size(); ++i) {
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
            } break;

        case IO_METHOD_USERPTR: {
            // init UPTR i/o
            if (!initIoUserptr())
                return false;

            // queue buffers
            for (int i = 0; i < m_frameMemory.size(); ++i) {
                struct v4l2_buffer buf;
                CLEAR(buf);
                buf.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory    = V4L2_MEMORY_USERPTR;
                buf.m.userptr = (unsigned long)m_frameMemory[i].start;
                buf.length    = m_frameMemory[i].length;
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
            } break;
    }
#elif defined(VD_BUILD_WIN_VFW)
    if (vfwResolveSymbols()) {
        capSetCallbackOnFrame(m_vHwnd, videoCallback);
        //capPreviewRate(m_vHwnd, 40);
        //capPreviewScale(m_vHwnd, TRUE);
        //capPreview(m_vHwnd, TRUE);
        //capDlgVideoFormat(m_vHwnd);
    }
#endif
    qDebug("VideoDevice::startCapturing: started");
    m_capturing = true;
    return true;
}

bool VideoDevice::stopCapturing()
{
    if (!m_capturing) {
        qDebug("VideoDevice::stopCapturing: already stopped");
        return true;
    }
#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor == -1) {
        qDebug("VideoDevice::stopCapturing: not opened");
        return false;
    }
    switch (m_linuxIO) {
        case IO_METHOD_NONE:
            // Device cannot capture frames
            break;

        case IO_METHOD_READ:
            // Nothing to do
            break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR: {
            // stop streaming
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            if (-1 == xioctl(VIDIOC_STREAMOFF, &type))
                perror("VIDIOC_STREAMOFF");

            // un-mmap/free userspace buffers
            if (m_linuxIO == IO_METHOD_MMAP) {
                for (int i = 0; i < m_frameMemory.size(); ++i) {
                    if (::munmap(m_frameMemory[i].start, m_frameMemory[i].length) != 0)
                        qDebug() << "VideoDevice::stopCapturing: unable to munmap";
                }
            } else if (m_linuxIO == IO_METHOD_USERPTR) {
                for (int i = 0; i < m_frameMemory.size(); ++i)
                    ::free(m_frameMemory[i].start);
            }

            // release device buffers
            struct v4l2_requestbuffers req;
            CLEAR(req);
            req.count  = 0; // this is the key for releasing the buffers
            req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            req.memory = (m_linuxIO == IO_METHOD_MMAP) ? V4L2_MEMORY_MMAP : V4L2_MEMORY_USERPTR;
            if (-1 == xioctl(VIDIOC_REQBUFS, &req))
                perror("VIDIOC_REQBUFS (stop)");
            m_frameMemory.clear();
            } break;
    }
#elif defined(VD_BUILD_WIN_VFW)
    if (m_vHwnd) {
        capPreview(m_vHwnd, FALSE);
        capDriverDisconnect(m_vHwnd);
        DestroyWindow(m_vHwnd);
        m_vHwnd = 0;
        delete m_vWidget;
        m_vWidget = 0;
    }
    WINTODO;
#endif
    qDebug("VideoDevice::stopCapturing: stopped");
    m_capturing = false;
    return true;
}

bool VideoDevice::captureFrame()
{
    if (!m_capturing) {
        qDebug("VideoDevice::acquireFrame: start stream before acquiring frames");
        return false;
    }
#if defined(VD_BUILD_LINUX_V4L2)
    if (m_videoFileDescriptor == -1) {
        qDebug("VideoDevice::acquireFrame: not opened");
        return false;
    }

    switch (m_linuxIO) {
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
            // dequeue a buffer
            struct v4l2_buffer v4l2buffer;
            CLEAR(v4l2buffer);
            v4l2buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4l2buffer.memory = V4L2_MEMORY_MMAP;
            if (-1 == xioctl (VIDIOC_DQBUF, &v4l2buffer)) {
                qDebug() << "VideoDevice::acquireFrame:" << m_info.filePath << "MMAPed getFrame failed";
                switch (errno) {
                    case EAGAIN:
                        qDebug() << "VideoDevice::acquireFrame:" << m_info.filePath << " MMAPed getFrame failed: EAGAIN. Pointer: ";
                        return false;

                    case EIO: // Could ignore EIO, see spec. fall through
                    default:
                        perror("VIDIOC_DQBUF (mmap)");
                        return false;
                }
            }

            // copy data to the ImageBuffer
            memcpy(m_imageBuffer.data.data(), m_frameMemory[v4l2buffer.index].start, m_imageBuffer.data.size());

            // queue the buffer back
            if (-1 == xioctl(VIDIOC_QBUF, &v4l2buffer)) {
                perror("VIDIOC_QBUF (mmap)");
                return false;
            }
            } break;

        case IO_METHOD_USERPTR: {
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

            // find the related FrameMemory
            int i = 0;
            for (; i < m_frameMemory.size(); ++i)
                if (v4l2buffer.m.userptr == (unsigned long)m_frameMemory[i].start && v4l2buffer.length == m_frameMemory[i].length)
                    break;
            if (i >= m_frameMemory.size()) {
                qWarning("VideoDevice::acquireFrame: can't find the related FrameMemory");
                return false;
            }

            // TODO: copy memory?
            // ...

            // queue the buffer back
            if (-1 == xioctl(VIDIOC_QBUF, &v4l2buffer)) {
                perror("VIDIOC_QBUF (uptr)");
                return false;
            }
            } break;
    }
#elif defined(VD_BUILD_WIN_VFW)
    s_imageBuffer = &m_imageBuffer;
    if (!capGrabFrameNoStop(m_vHwnd)) {
        qDebug("VideoDevice::captureFrame: error capturing still frame");
        return false;
    }
#endif
    return true;
}

bool VideoDevice::getLastFrame(QImage * qimage) const
{
    // do NOT delete qimage here, as it is received as a parameter
    const int W = m_imageBuffer.size.width();
    const int H = m_imageBuffer.size.height();
    if (qimage->width() != W || qimage->height() != H)
        *qimage = QImage(W, H, QImage::Format_RGB32);

    //qDebug() << "VideoDevice::getImage: capturing in " << pixelFormatName(m_imageBuffer.pixelformat);
    const char * src = m_imageBuffer.data.constData();
    uchar * imgData = qimage->bits();
    int imgBytes = qimage->byteCount();
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
                for (int y = 0; y < H; y++) {
                    int dstep=(H - 1 - y) * W * 4;
                    for (int x = 0; x < W; x++) {
                        imgData[dstep]   = src[step];
                        imgData[dstep+1] = src[step+1];
                        imgData[dstep+2] = src[step+2];
                        imgData[dstep+3] = 255;
                        dstep+=4;
                        step+=3;
                    }
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

                uint *p = (uint*)qimage->bits();
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

                        //uint *p = (uint*)qimage->scanLine(y)+x;
                        *p++ = qRgba(r,g,b,255);
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
    if (m_frameOperations & (FO_AutoBrightnessContrast | FO_AutoColorCorrection)) {
        ///unsigned long long result=0;
        unsigned long long R=0, G=0, B=0 /*, A=0*/ /*, global=0*/;
        int Rmax=0, Gmax=0, Bmax=0, /*Amax=0,*/ globalmax=0;
        int Rmin=255, Gmin=255, Bmin=255, /*Amin=255,*/ globalmin=255;
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
        //global = R + G + B;
        // Finds overall minimum and maximum intensity
        if (Rmin > Gmin) globalmin = Gmin; else globalmin = Rmin; if (Bmin < globalmin) globalmin = Bmin;
        if (Rmax > Gmax) globalmax = Rmax; else globalmax = Gmax; if (Bmax > globalmax) globalmax = Bmax;
        // If no color correction should be performed, simply level all the intensities so they're just the same.
        // In fact color correction should use the R, G and B variables to detect color deviation and "bump up" the saturation,
        // but it's computationally more expensive and the current way returns better results to the user.
        if (!(m_frameOperations & FO_AutoColorCorrection)) {
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

        /*qDebug() << " R: " << R << " G: " << G << " B: " << B << " A: " << A << " global: " << global <<
                " Rmin: " << Rmin << " Gmin: " << Gmin << " Bmin: " << Bmin << " Amin: " << Amin << " globalmin: " << globalmin <<
                " Rmax: " << Rmax << " Gmax: " << Gmax << " Bmax: " << Bmax << " Amax: " << Amax << " globalmax: " << globalmax ;
*/
        for(int i = 0; i < imgBytes; i += 4) {
            imgData[i]   = (imgData[i]   - Rmin) * 255 / (Rrange);
            imgData[i+1] = (imgData[i+1] - Gmin) * 255 / (Grange);
            imgData[i+2] = (imgData[i+2] - Bmin) * 255 / (Brange);
            //			bits[loop+3] = (bits[loop+3] - Amin) * 255 / (Arange);
        }
    }
    return true;
}

quint32 VideoDevice::frameOperations() const
{
    return m_frameOperations;
}

void VideoDevice::setFrameOperations(quint32 operations)
{
    m_frameOperations = operations;
}

bool VideoDevice::queryDeviceProperties()
{
    // reset caps
    m_attributes = DA_None;
    m_inputs.clear();

#if defined(VD_BUILD_LINUX_V4L2)
    m_linuxDriver = LINUX_DRIVER_NONE;
    m_linuxIO = IO_METHOD_NONE;
    struct v4l2_capability V4L2_capabilities;
    CLEAR(V4L2_capabilities);
    if (-1 != xioctl(VIDIOC_QUERYCAP, &V4L2_capabilities)) {
        // check that is a CaptureDevice
        if (!(V4L2_capabilities.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            qDebug() << "VideoDevice::queryDeviceProperties:" << m_info.filePath << "is not a video capture device.";
            return false;
        }
        qDebug() << "VideoDevice::queryDeviceProperties:" << m_info.filePath << "is a V4L2 device.";
        m_linuxDriver = LINUX_DRIVER_V4L2;
        m_attributes |= DA_Capture;
        m_info.prettyName = QString::fromLocal8Bit((const char*)V4L2_capabilities.card);

        // find out acquiral mechanism, based on capabilities
        if (V4L2_capabilities.capabilities & V4L2_CAP_READWRITE) {
            m_linuxIO = IO_METHOD_READ;
            m_attributes |= DA_IORead;
        }
        if (V4L2_capabilities.capabilities & V4L2_CAP_STREAMING) {
            m_linuxIO = IO_METHOD_MMAP;
            m_attributes |= DA_IOStream;
        }
        if (V4L2_capabilities.capabilities & V4L2_CAP_ASYNCIO)
            m_attributes |= DA_IOAsync;

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
        m_maxSize = QSize(format.fmt.pix.width, format.fmt.pix.height);

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
        m_minSize = QSize(format.fmt.pix.width, format.fmt.pix.height);

        // Buggy driver paranoia
        /* min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
            fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
            fmt.fmt.pix.sizeimage = min;
        m_buffer_size=fmt.fmt.pix.sizeimage ;*/

        // Recreate the inputs vector
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
            m_inputs.append(input);
            qDebug() << "VideoDevice::queryDeviceProperties: Input " << iNumber << ": " << input.name << " (tuner: " << ((videoInput.type & V4L2_INPUT_TYPE_TUNER) != 0) << ")";
        }
    } else {
        // V4L-only drivers should return an EINVAL in errno to indicate they cannot handle V4L2 calls. Not every driver is compliant, so
        // it will try the V4L api even if the error code is different than expected.
        qDebug() << "VideoDevice::queryDeviceProperties: " << m_info.filePath << " is not a V4L2 device.";
    }
    if (m_linuxDriver == LINUX_DRIVER_NONE) {
        qDebug() << "VideoDevice::queryDeviceProperties: cannot query properties.";
        return false;
    }

    // check if able to read from video
    if (m_linuxIO == IO_METHOD_NONE) {
        qDebug() << "VideoDevice::queryDeviceProperties: found no suitable input/output method for " << m_info.filePath;
        return false;
    }

    // print out supported pixel format/s
    detectPixelFormats();

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
        switch (errno) {
            case EINVAL: break;  // Cropping not supported.
            default:     break;  // Errors ignored.
        }
    }
#elif defined(VD_BUILD_WIN_VFW)
    VideoInput input1;
    input1.name = m_info.prettyName;
    input1.hastuner = false;
    m_inputs.append(input1);
#endif

    // device can be used
    return true;
}

void VideoDevice::detectPixelFormats()
{
#if defined(VD_BUILD_LINUX_V4L2)
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
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

            // fall back to classic V4L1 detection
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
    }
#elif defined(VD_BUILD_WIN_VFW)
    WINTODO;
#endif
}

PixelFormat VideoDevice::setPixelFormat(PixelFormat newformat) const
{
    // change the pixel format for the video device
#if defined(VD_BUILD_LINUX_V4L2)
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
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
    }
#elif defined(VD_BUILD_WIN_VFW)
    Q_UNUSED(newformat)
    //WINTODO;
#else
    Q_UNUSED(newformat)
    NEWTODO;
#endif
    return PIXELFORMAT_NONE;
}

bool VideoDevice::setInputParameters()
{
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

PixelFormat VideoDevice::pixelFormatFromPlatform(int platform) const
{
#if defined(VD_BUILD_LINUX_V4L2)
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
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
    }
#elif defined(VD_BUILD_WIN_VFW)
    Q_UNUSED(platform)
    //WINTODO;
#else
    Q_UNUSED(platform)
    NEWTODO;
#endif
    return PIXELFORMAT_NONE;
}

int VideoDevice::pixelFormatToPlatform(PixelFormat pixelformat) const
{
#if defined(VD_BUILD_LINUX_V4L2)
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
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
    }
#elif defined(VD_BUILD_WIN_VFW)
    Q_UNUSED(pixelformat)
    //WINTODO;
#else
    Q_UNUSED(pixelformat)
    NEWTODO;
#endif
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
#if defined(VD_BUILD_LINUX_V4L2)
    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2:
            switch (pixelformat) {
                case 0                          : returnvalue = pixelFormatName(PIXELFORMAT_NONE);      break;

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
    }
#elif defined(VD_BUILD_WIN_VFW)
    Q_UNUSED(pixelformat)
    //WINTODO;
#else
    Q_UNUSED(pixelformat)
    NEWTODO;
#endif
    return returnvalue;
}

#if defined(VD_BUILD_LINUX_V4L2)
bool VideoDevice::detectSignalStandards() const
{
    if (m_videoFileDescriptor == -1) {
        qWarning("VideoDevice::detectSignalStandards: device not opened.");
        return false;
    }

    switch (m_linuxDriver) {
        case LINUX_DRIVER_NONE:
            break;

        case LINUX_DRIVER_V4L2: {
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
    }
    return true;
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
    if (m_videoFileDescriptor == -1) {
        qDebug("VideoDevice::initIoRead: not opened");
        return false;
    }

    // don't allocate anything: data will be copied straight to the ImageBuffer
    m_frameMemory.clear();
    return true;
}

bool VideoDevice::initIoMmap()
{
    if (m_videoFileDescriptor == -1) {
        qDebug("VideoDevice::initIoMmap: not opened");
        return false;
    }

    // request some MMAP buffers
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count  = IO_DATA_BUFFERS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(VIDIOC_REQBUFS, &req)) {
        if (errno == EINVAL) {
            qDebug() << "VideoDevice::initIoMmap:" << m_info.filePath << "does not support memory mapping";
            return false;
        } else {
            perror("VIDIOC_REQBUFS (mmap)");
            return false;
        }
    }
    if (req.count < IO_DATA_BUFFERS) {
        qDebug() << "VideoDevice::initIoMmap: insufficient buffer memory on " << m_info.filePath;
        return false;
    }

    // for each requested buffer
    m_frameMemory.clear();
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

        // create the FrameMemory companion
        FrameMemory m;
        m.length = vBuffer.length;
        m.start = (uchar *)start;
        m_frameMemory.append(m);
    }

    // Makes the imagesize.data buffer size equal to the rawbuffer size
    if (m_imageBuffer.data.size() != (int)m_frameMemory[0].length) {
        qWarning("VideoDevice::initIoMmap: image buffer size %d was different than buffer's one %d. adapted.", m_imageBuffer.data.size(), (int)m_frameMemory[0].length);
        m_imageBuffer.data.resize(m_frameMemory[0].length);
    }
    qDebug() << "VideoDevice::initIoMmap: image buffer size" << m_imageBuffer.data.size();
    return true;
}

bool VideoDevice::initIoUserptr()
{
    if (m_videoFileDescriptor == -1) {
        qDebug("VideoDevice::initIoUserptr: not opened");
        return false;
    }

    // request some USERPTR buffers
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count  = IO_DATA_BUFFERS;
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;
    if (-1 == xioctl(VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            qDebug() << "VideoDevice::initIoUserptr:" << m_info.filePath << "does not support memory mapping";
            return false;
        } else {
            perror("VIDIOC_REQBUFS (uptr)");
            return false;
        }
    }
    if (req.count < IO_DATA_BUFFERS) {
        qDebug() << "VideoDevice::initIoMmap: insufficient buffer memory on " << m_info.filePath;
        return false;
    }

    // for each requested buffer
    m_frameMemory.clear();
    for (unsigned int i = 0; i < req.count; ++i) {
        // create the FrameMemory with userspace memory allocation
        FrameMemory m;
        m.length = m_imageBuffer.data.size();
        m.start = (uchar *)malloc(m.length);
        if (!m.start) {
            qDebug("VideoDevice::initIoUserptr: Out of memory");
            return false;
        }
        m_frameMemory.append(m);
    }
    return true;
}

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
                default: /*case V4L2_CTRL_TYPE_STRING:*/ break;
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


/// Obsolete Code
/// Obsolete Code
/// Obsolete Code

/*

typedef enum {
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
} signal_standard;

#define VIDEO_MODE_PAL_Nc  3
#define VIDEO_MODE_PAL_M   4
#define VIDEO_MODE_PAL_N   5
#define VIDEO_MODE_NTSC_JP 6

**** in VideoDevice class definition ****
    quint64 signalStandardCode(signal_standard standard);
    QString signalStandardName(signal_standard standard) const;
    QString signalStandardName(int standard) const;

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


float VideoDevice::getBrightness()
{
    if (m_currentInput < m_inputs.size() )
        return m_inputs[m_currentInput].getBrightness();
    else
        return 0;
}

float VideoDevice::setBrightness(float brightness)
{
    qDebug() << "(" << brightness << ") called.";
    m_inputs[m_currentInput].setBrightness(brightness); // Just to check bounds

    switch (m_linuxDriver)
    {
    case LINUX_DRIVER_V4L2:
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
    case LINUX_DRIVER_NONE:
        break;
    }
    return getBrightness();
}

float VideoDevice::getContrast()
{
    if (m_currentInput < m_inputs.size() )
        return m_inputs[m_currentInput].getContrast();
    else
        return 0;
}

float VideoDevice::setContrast(float contrast)
{
    qDebug() << "(" << contrast << ") called.";
    m_inputs[m_currentInput].setContrast(contrast); // Just to check bounds

    switch (m_linuxDriver)
    {
#if defined(VD_BUILD_LINUX_V4L2)
    case LINUX_DRIVER_V4L2:
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
    case LINUX_DRIVER_NONE:
        break;
    }
    return getContrast();
}

float VideoDevice::getSaturation()
{
    if (m_currentInput < m_inputs.size() )
        return m_inputs[m_currentInput].getSaturation();
    else
        return 0;
}

float VideoDevice::setSaturation(float saturation)
{
    qDebug() << "(" << saturation << ") called.";
    m_inputs[m_currentInput].setSaturation(saturation); // Just to check bounds

    switch (m_linuxDriver)
    {
#if defined(VD_BUILD_LINUX_V4L2)
    case LINUX_DRIVER_V4L2:
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
    case LINUX_DRIVER_NONE:
        break;
    }
    return getSaturation();
}

float VideoDevice::getWhiteness()
{
    if (m_currentInput < m_inputs.size() )
        return m_inputs[m_currentInput].getWhiteness();
    else
        return 0;
}

float VideoDevice::setWhiteness(float whiteness)
{
    qDebug() << "(" << whiteness << ") called.";
    m_inputs[m_currentInput].setWhiteness(whiteness); // Just to check bounds

    switch (m_linuxDriver)
    {
#if defined(VD_BUILD_LINUX_V4L2)
    case LINUX_DRIVER_V4L2:
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
    case LINUX_DRIVER_NONE:
        break;
    }
    return getWhiteness();
}

float VideoDevice::getHue()
{
    if (m_currentInput < m_inputs.size() )
        return m_inputs[m_currentInput].getHue();
    else
        return 0;
}

float VideoDevice::setHue(float hue)
{
    qDebug() << "(" << hue << ") called.";
    m_inputs[m_currentInput].setHue(hue); // Just to check bounds

    switch (m_linuxDriver)
    {
#if defined(VD_BUILD_LINUX_V4L2)
    case LINUX_DRIVER_V4L2:
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
    case LINUX_DRIVER_NONE:
        break;
    }
    return getHue();
}

quint64 VideoDevice::signalStandardCode(signal_standard standard)
{
    switch (m_linuxDriver)
    {
#if defined(VD_BUILD_LINUX_V4L2)
    case LINUX_DRIVER_V4L2:
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
    case LINUX_DRIVER_NONE:
        return STANDARD_UNKNOWN;
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
    switch (m_linuxDriver) {
#if defined(VD_BUILD_LINUX_V4L2)
        case LINUX_DRIVER_V4L2:
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
        case LINUX_DRIVER_NONE:
            break;
    }
    return returnvalue;
}
*/

} // namespace VideoCapture
