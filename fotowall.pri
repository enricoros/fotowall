# Fotowall input files
HEADERS += 3rdparty/gsuggest.h \
    CPixmap.h \
    CroppingDialog.h \
    CroppingWidget.h \
    Desk.h \
    ExactSizeDialog.h \
    ExportWizard.h \
    FlickrInterface.h \
    GlowEffectDialog.h \
    GlowEffectWidget.h \
    GroupBoxWidget.h \
    MainWindow.h \
    ModeInfo.h \
    PictureEffect.h \
    RenderOpts.h \
    VersionCheckDialog.h \
    VideoProvider.h \
    XmlRead.h \
    XmlSave.h

SOURCES += 3rdparty/gsuggest.cpp \
    CPixmap.cpp \
    CroppingDialog.cpp \
    CroppingWidget.cpp \
    Desk.cpp \
    ExactSizeDialog.cpp \
    ExportWizard.cpp \
    FlickrInterface.cpp \
    GlowEffectDialog.cpp \
    GlowEffectWidget.cpp \
    GroupBoxWidget.cpp \
    MainWindow.cpp \
    ModeInfo.cpp \
    VersionCheckDialog.cpp \
    VideoProvider.cpp \
    XmlRead.cpp \
    XmlSave.cpp \
    main.cpp

FORMS += CroppingDialog.ui \
    ExactSizeDialog.ui \
    ExportWizard.ui \
    GlowEffectDialog.ui \
    MainWindow.ui \
    VersionCheckDialog.ui

RESOURCES += fotowall.qrc

# More input files
include(items/items.pri)
include(frames/frames.pri)
include(3rdparty/richtextedit/richtextedit.pri)
include(3rdparty/videocapture/videocapture.pri)

# Translations of the core Fotowall files
TRANSLATIONS += translations/fotowall_de.ts \
    translations/fotowall_fr.ts \
    translations/fotowall_it.ts \
    translations/fotowall_pl.ts \
    translations/fotowall_pt_BR.ts
