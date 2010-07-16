VPATH += $$PWD
DEPENDPATH += $$PWD

!contains(CONFIG, no-likeback): {
    DEFINES += HAS_LIKEBACK

    HEADERS += \
        LikeBack.h \
        LikeBack_p.h \
        LikeBackDialog.h

    SOURCES += \
        LikeBack.cpp \
        LikeBackDialog.cpp

    FORMS += \
        LikeBackDialog.ui

    RESOURCES += \
        likebackfrontend.qrc
} else: {
    message("3rdparty/likebackfrontend won't be compiled")
    DEFINES -= HAS_LIKEBACK
}
