VPATH += $$PWD
DEPENDPATH += $$PWD

macx: CONFIG += NO_VIDEOCAPTURE

!contains(CONFIG, NO_VIDEOCAPTURE): {
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
