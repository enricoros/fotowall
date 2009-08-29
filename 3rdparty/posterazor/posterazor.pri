VPATH = $$PWD
INCLUDEPATH += \
    $$PWD

SOURCES += \
    controller.cpp \
    wizard.cpp \
    paintcanvas.cpp \
    pdfwriter.cpp \
    posterazorcore.cpp \
    snapspinbox.cpp \
    types.cpp \
    wizardcontroller.cpp

HEADERS += \
    controller.h \
    imageloaderinterface.h \
    wizard.h \
    paintcanvas.h \
    paintcanvasinterface.h \
    pdfwriter.h \
    posterazorcore.h \
    snapspinbox.h \
    types.h \
    wizardcontroller.h

!contains (DEFINES, FREEIMAGE_LIB) {
    SOURCES += \
        imageloaderqt.cpp

    HEADERS += \
        imageloaderqt.h
}

FORMS += \
    wizard.ui

RESOURCES += \
    posterazor.qrc

TRANSLATIONS += \
    de.ts \
    en.ts \
    es.ts \
    fi.ts \
    fr.ts \
    it.ts \
    nl.ts \
    pl.ts \
    pt_BR.ts \
    pt_PT.ts \
    zh_CN.ts
