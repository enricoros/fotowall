TARGET = PosteRazor

DESTDIR = $$PWD

win32:DEFINES -= \
    UNICODE

macx:QMAKE_INFO_PLIST = \
    Info.plist

macx:ICON = \
    posterazor.icns

macx:CONFIG += \
    x86 ppc

macx:QMAKE_MAC_SDK = \
    /Developer/SDKs/MacOSX10.4u.sdk

macx:QMAKE_MACOSX_DEPLOYMENT_TARGET = \
    10.3

RC_FILE += \
    posterazor.rc

# Uncomment the following line in order to build PosteRazor with FreeImage
#DEFINES += FREEIMAGE_LIB

SOURCES += \
    mainwindow.cpp \
    main.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui \

contains (DEFINES, FREEIMAGE_LIB) {
    SOURCES += \
        imageloaderfreeimage.cpp

    HEADERS += \
        imageloaderfreeimage.h

    win32:INCLUDEPATH += \
        thirdparty/FreeImage/Dist

    win32:LIBS += \
        thirdparty/FreeImage/Dist/FreeImage.lib

    macx: INCLUDEPATH += \
        /usr/local/include

    unix:LIBS += \
        -lfreeimage
} else {
    contains(CONFIG, static) {
        QTPLUGIN += \
            qgif \
            qjpeg \
            qsvg \
            qtiff
    }
}

include (posterazor.pri)

!contains(CONFIG, build_pass) system(lrelease posterazor.pro)
