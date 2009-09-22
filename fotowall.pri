# Fotowall input files
HEADERS += \
    3rdparty/gsuggest.h \
    ButtonsDialog.h \
    CPixmap.h \
    Desk.h \
    ExactSizeDialog.h \
    ExportWizard.h \
    MainWindow.h \
    ModeInfo.h \
    PictureEffect.h \
    RenderOpts.h \
    XmlRead.h \
    XmlSave.h \
    tools/CroppingDialog.h \
    tools/CroppingWidget.h \
    tools/FlickrInterface.h \
    tools/GlowEffectDialog.h \
    tools/GlowEffectWidget.h \
    tools/GroupBoxWidget.h \
    tools/MetaXmlReader.h \
    tools/VersionCheckDialog.h \
    tools/VideoProvider.h

SOURCES += \
    3rdparty/gsuggest.cpp \
    ButtonsDialog.cpp \
    CPixmap.cpp \
    Desk.cpp \
    ExactSizeDialog.cpp \
    ExportWizard.cpp \
    MainWindow.cpp \
    ModeInfo.cpp \
    XmlRead.cpp \
    XmlSave.cpp \
    tools/CroppingDialog.cpp \
    tools/CroppingWidget.cpp \
    tools/FlickrInterface.cpp \
    tools/GlowEffectDialog.cpp \
    tools/GlowEffectWidget.cpp \
    tools/GroupBoxWidget.cpp \
    tools/MetaXmlReader.cpp \
    tools/VersionCheckDialog.cpp \
    tools/VideoProvider.cpp \
    main.cpp

FORMS += \
    ExactSizeDialog.ui \
    ExportWizard.ui \
    MainWindow.ui \
    tools/CroppingDialog.ui \
    tools/GlowEffectDialog.ui \
    tools/VersionCheckDialog.ui

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
