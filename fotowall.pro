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
    Desk.h \
    ExactSizeDialog.h \
    ExportWizard.h \
    GlowEffectDialog.h \
    GlowEffectWidget.h \
    FotoWall.h \
    ModeInfo.h \
    RenderOpts.h \
    XmlSave.h \
    XmlRead.h
SOURCES += main.cpp \
    CPixmap.cpp \
    Desk.cpp \
    ExactSizeDialog.cpp \
    ExportWizard.cpp \
    FotoWall.cpp \
    GlowEffectDialog.cpp \
    GlowEffectWidget.cpp \
    ModeInfo.cpp \
    XmlSave.cpp \
    XmlRead.cpp
FORMS += ExactSizeDialog.ui \
    ExportWizard.ui \
    FotoWall.ui \
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

macx {
    ICON = fotowall.icns
    CONFIG += x86 ppc
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
}

# handling static image plugins
win32|macx {
    contains(CONFIG, static)|contains(CONFIG, qt_no_framework) {
        DEFINES += STATIC_LINK
        QTPLUGIN += qgif \
            qjpeg \
            qsvg \
            qtiff
    }
}
