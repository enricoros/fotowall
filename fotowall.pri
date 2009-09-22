# Fotowall input files
#HEADERS += 3rdparty/gsuggest.h
#SOURCES += 3rdparty/gsuggest.cpp

SOURCES += main.cpp
RESOURCES += fotowall.qrc

# More input files
include(App/App.pri)
include(Desk/Desk.pri)
include(Frames/Frames.pri)
include(Shared/Shared.pri)
include(3rdparty/likebackfrontend/likebackfrontend.pri)
include(3rdparty/richtextedit/richtextedit.pri)
include(3rdparty/videocapture/videocapture.pri)

# Translations of the core Fotowall files
TRANSLATIONS += \
    translations/fotowall_de.ts \
    translations/fotowall_en.ts \
    translations/fotowall_fr.ts \
    translations/fotowall_it.ts \
    translations/fotowall_pl.ts \
    translations/fotowall_pt_BR.ts
