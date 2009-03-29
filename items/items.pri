VPATH += $$PWD
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# disabling the webkit component on windows, since I can't static link to it
win32 {
    DEFINES += SKIP_QTWEBKIT
} else {
    QT += webkit
    HEADERS += BrowserItem.h
    SOURCES += BrowserItem.cpp
}

HEADERS += ButtonItem.h \  
    ColorPickerItem.h \
    PictureItem.h \
    MirrorItem.h \
    PicturePropertiesItem.h \
    HighlightItem.h \
    HelpItem.h \
    CPixmap.h
SOURCES += ButtonItem.cpp \
    ColorPickerItem.cpp \
    PictureItem.cpp \
    MirrorItem.cpp \
    PicturePropertiesItem.cpp \
    HighlightItem.cpp \
    HelpItem.cpp \
    CPixmap.cpp
FORMS += PicturePropertiesItem.ui
