# Project Options
TEMPLATE = app
TARGET = fotowall
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

# FotoWall input files
HEADERS += 3rdparty/gsuggest.h \
    CPixmap.h \
    Desk.h \
    ExactSizeDialog.h \
    ExportWizard.h \
    FotoWall.h \
    GlowEffectDialog.h \
    GlowEffectWidget.h \
    ModeInfo.h \
    RenderOpts.h \
    XmlSave.h \
    XmlRead.h
SOURCES += 3rdparty/gsuggest.cpp \
    main.cpp \
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
