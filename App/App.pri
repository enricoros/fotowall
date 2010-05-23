HEADERS += \
    App/App.h \
    App/CanvasAppliance.h \
    App/ExactSizeDialog.h \
    App/FotowallFile.h \
    App/Hardware3DTest.h \
    App/HelpAppliance.h \
    App/HelpItem.h \
    App/HomeAppliance.h \
    App/HomeScene.h \
    App/OnlineServices.h \
    App/PictureSearchWidget.h \
    App/SceneView.h \
    App/Settings.h \
    App/UrlHistoryBox.h \
    App/VersionCheckDialog.h \
    App/Workflow.h

SOURCES += \
    App/App.cpp \
    App/CanvasAppliance.cpp \
    App/ExactSizeDialog.cpp \
    App/FotowallFile.cpp \
    App/Hardware3DTest.cpp \
    App/HelpAppliance.cpp \
    App/HelpItem.cpp \
    App/HomeAppliance.cpp \
    App/HomeScene.cpp \
    App/OnlineServices.cpp \
    App/PictureSearchWidget.cpp \
    App/SceneView.cpp \
    App/Settings.cpp \
    App/UrlHistoryBox.cpp \
    App/VersionCheckDialog.cpp \
    App/Workflow.cpp

FORMS += \
    App/CanvasAppliance.ui \
    App/ExactSizeDialog.ui \
    App/HelpAppliance.ui \
    App/PictureSearchWidget.ui \
    App/VersionCheckDialog.ui

contains(CONFIG, mobile-form-factor): {
    HEADERS += App/MainWindow_s60.h
    SOURCES += App/MainWindow_s60.cpp
    FORMS += App/MainWindow_s60.ui
} else: {
    HEADERS += App/MainWindow.h
    SOURCES += App/MainWindow.cpp
    FORMS += App/MainWindow.ui
}

!contains(CONFIG, no-export): {
    DEFINES += HAS_EXPORTDIALOG
    HEADERS += \
        App/ExportWizard.h

    SOURCES += \
        App/ExportWizard.cpp

    FORMS += \
        App/ExportWizard.ui
} else: {
    message("ExportWizard (and PosteRazor) won't be compiled")
    DEFINES -= HAS_EXPORTDIALOG
}

!contains(CONFIG, no-wordcloud-appliance): {
    DEFINES += HAS_WORDCLOUD_APPLIANCE
    HEADERS += \
        App/WordcloudAppliance.h \
        App/WordcloudSidebar.h

    SOURCES += \
        App/WordcloudAppliance.cpp \
        App/WordcloudSidebar.cpp

    FORMS += \
        App/WordcloudAppliance.ui \
        App/WordcloudSidebar.ui
} else: {
    message("Wordcloud appliance won't be compiled")
    DEFINES -= HAS_WORDCLOUD_APPLIANCE
}
