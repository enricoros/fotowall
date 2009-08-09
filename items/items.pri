VPATH += $$PWD
DEPENDPATH += $$PWD

# disabling the webkit component on windows, since I can't static link to it
# disabled webkit on every platform - Enrico 20090410
# DEFINES += USE_QTWEBKIT
# QT += webkit
# HEADERS += BrowserItem.h
# SOURCES += BrowserItem.cpp
HEADERS += AbstractContent.h \
    AbstractDisposeable.h \
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
    BezierTextContent.h \
    BezierTextProperties.h \
    VideoContent.h \
    VideoProvider.h \
    CornerItem.h \
    WebContentSelectorItem.h
SOURCES += AbstractContent.cpp \
    AbstractDisposeable.cpp \
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
    BezierTextContent.cpp \
    BezierTextProperties.cpp \
    VideoContent.cpp \
    VideoProvider.cpp \
    CornerItem.cpp \
    WebContentSelectorItem.cpp
FORMS += AbstractProperties.ui \
    PictureProperties.ui \
    WebContentSelectorItem.ui
