VPATH += $$PWD
DEPENDPATH += $$PWD

!contains(CONFIG, no-webcam): {
    DEFINES += HAS_VIDEOCAPTURE

    HEADERS += $$PWD/bayer.h \
               $$PWD/sonix_compress.h \
               $$PWD/VideoDevice.h \
               $$PWD/VideoInput.h

    SOURCES += $$PWD/bayer.cpp \
               $$PWD/sonix_compress.cpp \
               $$PWD/VideoDevice.cpp \
               $$PWD/VideoInput.cpp
} else: {
    message("3rdparty/VideoCapture won't be compiled")
    DEFINES -= HAS_VIDEOCAPTURE
}
