# disabling the webkit component on windows, since I can't static link to it
# disabled webkit on every platform - Enrico 20090410
# DEFINES += USE_QTWEBKIT
# QT += webkit
# HEADERS += Desk/BrowserItem.h
# SOURCES += Desk/BrowserItem.cpp
HEADERS += \
    Desk/AbstractConfig.h \
    Desk/AbstractContent.h \
    Desk/AbstractDisposeable.h \
    Desk/BezierCubicItem.h \
    Desk/ButtonItem.h \
    Desk/CornerItem.h \
    Desk/Desk.h \
    Desk/HelpItem.h \
    Desk/HighlightItem.h \
    Desk/MirrorItem.h \
    Desk/PictureConfig.h \
    Desk/PictureContent.h \
    Desk/PictureProperties.h \
    Desk/PropertyEditors.h \
    Desk/SelectionProperties.h \
    Desk/StyledButtonItem.h \
    Desk/TextConfig.h \
    Desk/TextContent.h \
    Desk/TextProperties.h \
    Desk/DeskViewContent.h \
    Desk/WebContentSelectorItem.h \
    Desk/WebcamContent.h

SOURCES += \
    Desk/AbstractConfig.cpp \
    Desk/AbstractContent.cpp \
    Desk/AbstractDisposeable.cpp \
    Desk/BezierCubicItem.cpp \
    Desk/ButtonItem.cpp \
    Desk/CornerItem.cpp \
    Desk/Desk.cpp \
    Desk/HelpItem.cpp \
    Desk/HighlightItem.cpp \
    Desk/MirrorItem.cpp \
    Desk/PictureConfig.cpp \
    Desk/PictureContent.cpp \
    Desk/PictureProperties.cpp \
    Desk/PropertyEditors.cpp \
    Desk/SelectionProperties.cpp \
    Desk/StyledButtonItem.cpp \
    Desk/TextConfig.cpp \
    Desk/TextContent.cpp \
    Desk/TextProperties.cpp \
    Desk/DeskViewContent.cpp \
    Desk/WebContentSelectorItem.cpp \
    Desk/WebcamContent.cpp

FORMS += \
    Desk/AbstractConfig.ui \
    Desk/PictureConfig.ui \
    Desk/PictureProperties.ui \
    Desk/TextProperties.ui \
    Desk/WebContentSelectorItem.ui
