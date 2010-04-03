VPATH += $$PWD
DEPENDPATH += $$PWD

macx: CONFIG += no-webcam
win32: CONFIG += no-webcam
symbian: CONFIG += no-webcam

!contains(CONFIG, no-webcam): {
    DEFINES += HAS_VIDEOCAPTURE

    HEADERS += bayer.h \
        sonix_compress.h \
        VideoDevice.h \
        VideoInput.h

    SOURCES += bayer.cpp \
        sonix_compress.cpp \
        VideoDevice.cpp \
        VideoInput.cpp
} else: {
    message("3rdparty/VideoCapture won't be compiled")
    DEFINES -= HAS_VIDEOCAPTURE
}
