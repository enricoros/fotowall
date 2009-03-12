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
    svg \
    webkit

# Input
HEADERS += Desk.h \
    FotoWall.h
SOURCES += main.cpp \
    Desk.cpp \
    FotoWall.cpp
FORMS += FotoWall.ui
RESOURCES += fotowall.qrc
TRANSLATIONS += translations/fotowall_en.ts \
    translations/fotowall_it.ts \
    translations/fotowall_fr.ts

# Components
include(items/items.pri)
include(frames/frames.pri)

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
win32 {
    CONFIG += embed_manifest_exe
    RC_FILE = fotowall.rc
}
