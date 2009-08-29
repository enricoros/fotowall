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
    AbstractConfig.h \
    ButtonItem.h \
    ColorPickerItem.h \
    HighlightItem.h \
    HelpItem.h \
    MirrorItem.h \
    PictureContent.h \
    PictureConfig.h \
    TextContent.h \
    TextConfig.h \
    VideoContent.h \
    VideoProvider.h \
    CornerItem.h \
    WebContentSelectorItem.h \
    BezierCubicItem.h \
    TextProperties.h \
    StyledButtonItem.h
SOURCES += AbstractContent.cpp \
    AbstractDisposeable.cpp \
    AbstractConfig.cpp \
    ButtonItem.cpp \
    ColorPickerItem.cpp \
    HighlightItem.cpp \
    HelpItem.cpp \
    MirrorItem.cpp \
    PictureContent.cpp \
    PictureConfig.cpp \
    TextContent.cpp \
    TextConfig.cpp \
    VideoContent.cpp \
    VideoProvider.cpp \
    CornerItem.cpp \
    WebContentSelectorItem.cpp \
    BezierCubicItem.cpp \
    TextProperties.cpp \
    StyledButtonItem.cpp
FORMS += AbstractConfig.ui \
    PictureConfig.ui \
    WebContentSelectorItem.ui \
    TextProperties.ui
