HEADERS += \
    Shared/PlugGui/AbstractAppliance.h \
    Shared/PlugGui/Container.h \
    #Shared/PlugGui/Stacker.h \
    Shared/PictureServices/AbstractPictureService.h \
    Shared/PictureServices/FlickrPictureService.h \
    Shared/PictureServices/GoogleImagesPictureService.h \
    Shared/AbstractDisposeable.h \
    Shared/AbstractScene.h \
    Shared/AbstractResourceProvider.h \
    Shared/BreadCrumbBar.h \
    Shared/ButtonsDialog.h \
    Shared/CPixmap.h \
    Shared/ColorPickerItem.h \
    Shared/CroppingDialog.h \
    Shared/CroppingWidget.h \
    Shared/GlowEffectDialog.h \
    Shared/GlowEffectWidget.h \
    Shared/GroupBoxWidget.h \
    Shared/HighlightItem.h \
    Shared/MetaXmlReader.h \
    Shared/PanePropertyEditor.h \
    Shared/PictureEffect.h \
    Shared/PixmapButton.h \
    Shared/PropertyEditors.h \
    Shared/RenderOpts.h \
    Shared/VideoProvider.h \
    Shared/Commands.h


SOURCES += \
    Shared/PlugGui/AbstractAppliance.cpp \
    Shared/PlugGui/Container.cpp \
    #Shared/PlugGui/Stacker.cpp \
    Shared/PictureServices/AbstractPictureService.cpp \
    Shared/PictureServices/FlickrPictureService.cpp \
    Shared/PictureServices/GoogleImagesPictureService.cpp \
    Shared/AbstractDisposeable.cpp \
    Shared/AbstractScene.cpp \
    Shared/BreadCrumbBar.cpp \
    Shared/ButtonsDialog.cpp \
    Shared/CPixmap.cpp \
    Shared/ColorPickerItem.cpp \
    Shared/CroppingDialog.cpp \
    Shared/CroppingWidget.cpp \
    Shared/GlowEffectDialog.cpp \
    Shared/GlowEffectWidget.cpp \
    Shared/GroupBoxWidget.cpp \
    Shared/HighlightItem.cpp \
    Shared/PanePropertyEditor.cpp \
    Shared/PixmapButton.cpp \
    Shared/PropertyEditors.cpp \
    Shared/MetaXmlReader.cpp \
    Shared/VideoProvider.cpp

FORMS += \
    Shared/CroppingDialog.ui \
    Shared/GlowEffectDialog.ui

!contains(CONFIG, mobile-ui): {
    HEADERS += \
        Shared/BlinkingToolButton.h

    SOURCES += \
        Shared/BlinkingToolButton.cpp
}
