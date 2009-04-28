TEMPLATE = app
TARGET = fotowall
DEPENDPATH += .
INCLUDEPATH += . 3rdparty
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
    XmlSave.h \
    XmlRead.h \
    ModeInfo.h \
    ExactSizeDialog.h
SOURCES += main.cpp \
    CPixmap.cpp \
    GlowEffectWidget.cpp \
    GlowEffectDialog.cpp \
    Desk.cpp \
    FotoWall.cpp \
    XmlSave.cpp \
    XmlRead.cpp \
    ModeInfo.cpp \
    ExactSizeDialog.cpp
FORMS += FotoWall.ui \
    GlowEffectDialog.ui \
    ExactSizeDialog.ui
RESOURCES += fotowall.qrc
TRANSLATIONS += translations/fotowall_de.ts \
    translations/fotowall_fr.ts \
    translations/fotowall_it.ts \
    translations/fotowall_pt_BR.ts

# Components
include(items/items.pri)
include(frames/frames.pri)
include(3rdparty/richtextedit/richtextedit.pri)
include(3rdparty/videocapture/videocapture.pri)
include(3rdparty/posterazor/posterazor.pri)
!contains(CONFIG, build_pass) system(lrelease 3rdparty/posterazor/posterazor.pri)

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
    RC_FILE = fotowall.rc
}

# handling static image plugins
contains(CONFIG, static)|contains(CONFIG, qt_no_framework) {
    QTPLUGIN += qgif \
        qjpeg \
        qsvg \
        qtiff
}
