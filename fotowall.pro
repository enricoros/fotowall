# Check Qt >= 4.4
contains(QT_VERSION, ^4\.[0-3]\..*) {
    message("Cannot build Qt Creator with Qt version $$QT_VERSION.")
    error("Use at least Qt 4.4.")
}

# Project Options
TEMPLATE = app
TARGET = fotowall
INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR = .build
OBJECTS_DIR = .build
RCC_DIR = .build
UI_DIR = .build
QT = core \
    gui \
    svg \
    network \
    xml

# OpenGL support
win32 {
    QT += opengl
    DEFINES += HAS_OPENGL
}

# FotoWall input files
HEADERS += 3rdparty/gsuggest.h \
    CPixmap.h \
    Desk.h \
    ExactSizeDialog.h \
    ExportWizard.h \
    FlickrInterface.h \
    FotoWall.h \
    GlowEffectDialog.h \
    GlowEffectWidget.h \
    GroupBoxWidget.h \
    CropingDialog.h \
    CropingWidget.h \
    ModeInfo.h \
    PictureEffect.h \
    RenderOpts.h \
    VersionCheckDialog.h \
    XmlSave.h \
    XmlRead.h
SOURCES += 3rdparty/gsuggest.cpp \
    main.cpp \
    CPixmap.cpp \
    Desk.cpp \
    ExactSizeDialog.cpp \
    ExportWizard.cpp \
    FlickrInterface.cpp \
    FotoWall.cpp \
    GlowEffectDialog.cpp \
    GlowEffectWidget.cpp \
    GroupBoxWidget.cpp \
    CropingDialog.cpp \
    CropingWidget.cpp \
    ModeInfo.cpp \
    VersionCheckDialog.cpp \
    XmlSave.cpp \
    XmlRead.cpp
FORMS += CropingDialog.ui \
    ExactSizeDialog.ui \
    ExportWizard.ui \
    FotoWall.ui \
    GlowEffectDialog.ui \
    VersionCheckDialog.ui
RESOURCES += fotowall.qrc
TRANSLATIONS += translations/fotowall_de.ts \
    translations/fotowall_fr.ts \
    translations/fotowall_it.ts \
    translations/fotowall_pl.ts \
    translations/fotowall_pt_BR.ts

# Sub-Components
include(items/items.pri)
include(frames/frames.pri)
include(3rdparty/richtextedit/richtextedit.pri)
include(3rdparty/videocapture/videocapture.pri)
include(3rdparty/posterazor/posterazor.pri)

# deployment on Linux
unix {
    target.path = /usr/bin
    icon.files = fotowall.png
    icon.path = /usr/share/pixmaps
    dfile.files = fotowall.desktop
    dfile.path = /usr/share/applications
    man.files = fotowall.1
    man.path = /usr/share/man/man1
    scripts.files = 3rdparty/fotowall.sh
    scripts.path = ~/.gnome2/nautilus-scripts
    INSTALLS += target \
        icon \
        dfile \
        man \
        scripts
}

# deployment on Windows
win32 {
    CONFIG += embed_manifest_exe
    RC_FILE = fotowall.rc
}

# deployment on Mac
macx {
    ICON = fotowall.icns
    CONFIG += x86 ppc
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
}

# static builds
win32|macx {
    contains(CONFIG, static)|contains(CONFIG, qt_no_framework) {
        DEFINES += STATIC_LINK
        QTPLUGIN += qgif \
            qjpeg \
            qsvg \
            qtiff
    }
}

# Translations (make ts; make qm)
LUPDATE = lupdate -silent -no-obsolete -no-ui-lines
LRELEASE = lrelease -silent -compress -removeidentical
ts.commands = ($$LUPDATE -pro translations.txt && $$LUPDATE 3rdparty/posterazor/posterazor.pri)
qm.commands = ($$LRELEASE translations/*.ts && $$LRELEASE 3rdparty/posterazor/*.ts)
QMAKE_EXTRA_TARGETS += ts qm
!exists(translations/fotowall_it.qm): {message("Compiling translations. To update strings type 'make ts', to recompile 'make qm'") system($$qm.commands)}
