# Fotowall input files
HEADERS += \
    3rdparty/gsuggest.h \
    ButtonsDialog.h \
    CPixmap.h \
    Desk.h \
    ExactSizeDialog.h \
    ExportWizard.h \
    FlickrInterface.h \
    GroupBoxWidget.h \
    MainWindow.h \
    MetaXmlReader.h \
    ModeInfo.h \
    PictureEffect.h \
    RenderOpts.h \
    VersionCheckDialog.h \
    VideoProvider.h \
    XmlRead.h \
    XmlSave.h \
    tools/CroppingDialog.h \
    tools/CroppingWidget.h \
    tools/GlowEffectDialog.h \
    tools/GlowEffectWidget.h

SOURCES += \
    3rdparty/gsuggest.cpp \
    ButtonsDialog.cpp \
    CPixmap.cpp \
    Desk.cpp \
    ExactSizeDialog.cpp \
    ExportWizard.cpp \
    FlickrInterface.cpp \
    GroupBoxWidget.cpp \
    MainWindow.cpp \
    MetaXmlReader.cpp \
    ModeInfo.cpp \
    VersionCheckDialog.cpp \
    VideoProvider.cpp \
    XmlRead.cpp \
    XmlSave.cpp \
    tools/CroppingDialog.cpp \
    tools/CroppingWidget.cpp \
    tools/GlowEffectDialog.cpp \
    tools/GlowEffectWidget.cpp \
    main.cpp

FORMS += \
    ExactSizeDialog.ui \
    ExportWizard.ui \
    MainWindow.ui \
    VersionCheckDialog.ui \
    tools/CroppingDialog.ui \
    tools/GlowEffectDialog.ui

RESOURCES += fotowall.qrc

# More input files
include(items/items.pri)
include(frames/frames.pri)
include(3rdparty/likebackfrontend/likebackfrontend.pri)
include(3rdparty/richtextedit/richtextedit.pri)
include(3rdparty/videocapture/videocapture.pri)

# Translations of the core Fotowall files
TRANSLATIONS += translations/fotowall_de.ts \
    translations/fotowall_en.ts \
    translations/fotowall_fr.ts \
    translations/fotowall_it.ts \
    translations/fotowall_pl.ts \
    translations/fotowall_pt_BR.ts
