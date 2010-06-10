# Check Qt >= 4.5
contains(QT_VERSION, ^4\.[0-4]\..*) {
    message("Cannot build Fotowall with Qt version $$QT_VERSION .")
    error("Use at least Qt 4.5.")
}
contains(QT_VERSION, ^4\.5\..*): message("Some features are not available with Qt $$QT_VERSION . Use Qt 4.6 or later.")

# Project Options
TEMPLATE = app
TARGET = fotowall
INCLUDEPATH += .
DEPENDPATH += .
MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build
UI_DIR = build
QT = core \
    gui \
    svg \
    network \
    xml

# use OpenGL where available
contains(QT_CONFIG, opengl)|contains(QT_CONFIG, opengles1)|contains(QT_CONFIG, opengles2) {
    QT += opengl
}

# enable features for mobile user interfaces
symbian|simulator: {
    CONFIG += mobile-ui
    CONFIG += no-wordcloud-appliance
    CONFIG += no-export
    CONFIG += no-likeback
    CONFIG += no-webcam
    CONFIG += no-translations
    QT -= opengl
}

# disable the Webcam source (only stable on linux)
macx|win32: CONFIG += no-webcam

# Fotowall input files
include(fotowall.pri)
# Posterazor input files
!contains(CONFIG, no-export): include(3rdparty/posterazor/posterazor.pri)

# deployment on Linux
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

# deployment on Mac
macx {
    ICON = fotowall.icns
    CONFIG += x86 ppc
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.3
}

# deployment on Symbian
symbian|simulator: {
    ICON = data/icon-s60.svg

    # QT += multimedia

    #CONFIG += mobility
    #MOBILITY = contacts messaging multimedia

    TARGET.CAPABILITY = \
        NetworkServices \
    #    ReadUserData \
    #    WriteUserData \
    #    LocalServices \
    #    UserEnvironment \
    #    ReadDeviceData \
    #    WriteDeviceData

    TARGET.UID3 = 0xe32c87ed
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
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
LUPDATE = lupdate -silent -no-obsolete -no-ui-lines -locations relative
LRELEASE = lrelease -silent -compress -removeidentical
ts.commands = ($$LUPDATE fotowall.pri && $$LUPDATE 3rdparty/posterazor/posterazor.pri)
qm.commands = ($$LRELEASE fotowall.pri && $$LRELEASE 3rdparty/posterazor/posterazor.pri)
QMAKE_EXTRA_TARGETS += ts qm
!exists(translations/fotowall_it.qm): {message("Compiling translations. To update strings type 'make ts', to recompile 'make qm'") system($$qm.commands)}
