VPATH += $$PWD
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += bayer.h \
    sonix_compress.h \
    videodevice.h \
    videodevicemodelpool.h \
    videodevicepool.h \
    videoinput.h

SOURCES += bayer.cpp \
    sonix_compress.cpp \
    videodevice.cpp \
    videodevicemodelpool.cpp \
    videodevicepool.cpp \
    videoinput.cpp
