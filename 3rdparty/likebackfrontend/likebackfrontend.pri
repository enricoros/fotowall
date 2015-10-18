VPATH += $$PWD
DEPENDPATH += $$PWD

!contains(CONFIG, no-likeback): {
    DEFINES += HAS_LIKEBACK

    HEADERS += \
        $$PWD/LikeBack.h \
        $$PWD/LikeBack_p.h \
        $$PWD/LikeBackDialog.h

    SOURCES += \
        $$PWD/LikeBack.cpp \
        $$PWD/LikeBackDialog.cpp

    FORMS += \
        $$PWD/LikeBackDialog.ui

    RESOURCES += \
        $$PWDlikebackfrontend.qrc
} else: {
    message("3rdparty/likebackfrontend won't be compiled")
    DEFINES -= HAS_LIKEBACK
}
