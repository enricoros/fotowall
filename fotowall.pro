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
    network \
    xml

# Input
HEADERS += CPixmap.h \
    GlowEffectWidget.h \
    GlowEffectDialog.h \
    Desk.h \
    FotoWall.h \
    RenderOpts.h \
    Selection.h \
    XmlSave.h \
    XmlRead.h
SOURCES += main.cpp \
    CPixmap.cpp \
    GlowEffectWidget.cpp \
    GlowEffectDialog.cpp \
    Desk.cpp \
    FotoWall.cpp \
    Selection.cpp \
    XmlSave.cpp \
    XmlRead.cpp
FORMS += FotoWall.ui \
    GlowEffectDialog.ui
RESOURCES += fotowall.qrc
TRANSLATIONS += translations/fotowall_de.ts \
    translations/fotowall_fr.ts \
    translations/fotowall_it.ts \
    translations/fotowall_pt_BR.ts

# Components
include(items/items.pri)
include(frames/frames.pri)
include(3rdparty/richtextedit/richtextedit.pri)

# installation on Linux
unix {
    target.path = /usr/bin
    icon.files = fotowall.png
    icon.path = /usr/share/pixmaps
    dfile.files = fotowall.desktop
    dfile.path = /usr/share/applications
    man.files = fotowall.1
    man.path = /usr/share/man/man1
    INSTALLS += target \
        icon \
        dfile \
        man
}

# deployment on Windows
win32 {
    CONFIG += embed_manifest_exe
    QTPLUGIN += qgif \
        qjpeg \
        qsvg \
        qtiff
    RC_FILE = fotowall.rc
}
