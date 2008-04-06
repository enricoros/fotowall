TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .
MOC_DIR = .bin
OBJECTS_DIR = .bin
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
