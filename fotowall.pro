TEMPLATE = app
TARGET = fotowall
DEPENDPATH += .
INCLUDEPATH += .
MOC_DIR = .bin
OBJECTS_DIR = .bin
RCC_DIR = .bin
UI_DIR = .bin
QT = core \
    gui \
    svg

# Input
HEADERS += ColorPickerItem.h \
    FotoWall.h \
    PictureItem.h \
    FWScene.h \
    frames/Frame.h \
    frames/StandardFrame.h \
    frames/HeartFrame.h \
    frames/PlasmaFrame.h \
    ButtonItem.h
SOURCES += main.cpp \
    ColorPickerItem.cpp \
    FotoWall.cpp \
    PictureItem.cpp \
    FWScene.cpp \
    frames/Frame.cpp \
    frames/StandardFrame.cpp \
    frames/HeartFrame.cpp \
    frames/PlasmaFrame.cpp \
    ButtonItem.cpp
FORMS += FotoWall.ui
RESOURCES += fotowall.qrc
TRANSLATIONS += translations/fotowall_en.ts \
    translations/fotowall_it.ts \
    translations/fotowall_fr.ts

# installation on Linux
unix { 
    target.path = /usr/bin
    icon.files = data/fotowall.png
    icon.path = /usr/share/pixmaps
    dfile.files = data/fotowall.desktop
    dfile.path = /usr/share/applications
    INSTALLS += target \
        icon \
        dfile
}

# deployment on Windows
win32:RC_FILE = fotowall.rc
