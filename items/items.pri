VPATH += $$PWD
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# disabling the webkit component on windows, since I can't static link to it
win32:DEFINES += SKIP_QTWEBKIT
else { 
    QT += webkit
    HEADERS += BrowserItem.h
    SOURCES += BrowserItem.cpp
}
HEADERS += AbstractProperties.h \
    ButtonItem.h \
    ColorPickerItem.h \
    PictureContent.h \
    MirrorItem.h \
    PictureProperties.h \
    HighlightItem.h \
    HelpItem.h \
    AbstractContent.h \
    TextContent.h \
    TextProperties.h
SOURCES += AbstractProperties.cpp \
    ButtonItem.cpp \
    ColorPickerItem.cpp \
    PictureContent.cpp \
    MirrorItem.cpp \
    PictureProperties.cpp \
    HighlightItem.cpp \
    HelpItem.cpp \
    AbstractContent.cpp \
    TextContent.cpp \
    TextProperties.cpp
FORMS += AbstractProperties.ui \
    PictureProperties.ui
