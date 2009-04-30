VPATH += $$PWD
DEPENDPATH += $$PWD

# disabling the webkit component on windows, since I can't static link to it
# disabled webkit on every platform - Enrico 20090410
# DEFINES += USE_QTWEBKIT
# QT += webkit
# HEADERS += BrowserItem.h
# SOURCES += BrowserItem.cpp
HEADERS += AbstractContent.h \
    AbstractProperties.h \
    ButtonItem.h \
    ColorPickerItem.h \
    HighlightItem.h \
    HelpItem.h \
    MirrorItem.h \
    PictureContent.h \
    PictureProperties.h \
    TextContent.h \
    TextProperties.h \
    VideoContent.h \
    VideoProvider.h \
    CornerItem.h
SOURCES += AbstractContent.cpp \
    AbstractProperties.cpp \
    ButtonItem.cpp \
    ColorPickerItem.cpp \
    HighlightItem.cpp \
    HelpItem.cpp \
    MirrorItem.cpp \
    PictureContent.cpp \
    PictureProperties.cpp \
    TextContent.cpp \
    TextProperties.cpp \
    VideoContent.cpp \
    VideoProvider.cpp \
    CornerItem.cpp
FORMS += AbstractProperties.ui \
    PictureProperties.ui
