VPATH += $$PWD
DEPENDPATH += $$PWD

# disabling the webkit component on windows, since I can't static link to it
# disabled webkit on every platform - Enrico 20090410
# DEFINES += USE_QTWEBKIT
# QT += webkit
# HEADERS += BrowserItem.h
# SOURCES += BrowserItem.cpp
HEADERS += \
    AbstractConfig.h \
    AbstractContent.h \
    AbstractDisposeable.h \
    BezierCubicItem.h \
    ButtonItem.h \
    ColorPickerItem.h \
    CornerItem.h \
    HelpItem.h \
    HighlightItem.h \
    MirrorItem.h \
    PictureConfig.h \
    PictureContent.h \
    PictureProperties.h \
    PropertyEditors.h \
    StyledButtonItem.h \
    TextConfig.h \
    TextContent.h \
    TextProperties.h \
    WebContentSelectorItem.h \
    WebcamContent.h
SOURCES += \
    AbstractConfig.cpp \
    AbstractContent.cpp \
    AbstractDisposeable.cpp \
    BezierCubicItem.cpp \
    ButtonItem.cpp \
    ColorPickerItem.cpp \
    CornerItem.cpp \
    HelpItem.cpp \
    HighlightItem.cpp \
    MirrorItem.cpp \
    PictureConfig.cpp \
    PictureContent.cpp \
    PictureProperties.cpp \
    PropertyEditors.cpp \
    StyledButtonItem.cpp \
    TextConfig.cpp \
    TextContent.cpp \
    TextProperties.cpp \
    WebContentSelectorItem.cpp \
    WebcamContent.cpp
FORMS += \
    AbstractConfig.ui \
    PictureConfig.ui \
    PictureProperties.ui \
    TextProperties.ui \
    WebContentSelectorItem.ui
