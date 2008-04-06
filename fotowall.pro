TEMPLATE = app
TARGET = fotowall
DEPENDPATH += .
INCLUDEPATH += .
MOC_DIR = .bin
OBJECTS_DIR = .bin
RCC_DIR = .bin
UI_DIR = .bin

# Input
HEADERS += \
 ColorPickerItem.h \
 FotoWall.h \
 FWFoto.h \
 FWScene.h

SOURCES += \
 main.cpp \
 ColorPickerItem.cpp \
 FotoWall.cpp \
 FWFoto.cpp \
 FWScene.cpp

FORMS += \
 FotoWall.ui

RESOURCES += \
 fotowall.qrc

#win32: RC_FILE = fotowall.rc

# default path for installation
target.path = /usr/bin
icon.files = data/fotowall.png
icon.path = /usr/share/pixmaps
dfile.files = data/fotowall.desktop
dfile.path = /usr/share/applications
INSTALLS += target icon dfile
