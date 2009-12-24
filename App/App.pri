HEADERS += \
    App/App.h \
    App/CanvasAppliance.h \
    App/ExactSizeDialog.h \
    App/ExportWizard.h \
    App/FotowallFile.h \
    App/Hardware3DTest.h \
    App/HelpAppliance.h \
    App/HelpItem.h \
    App/HomeAppliance.h \
    App/HomeScene.h \
    App/MainWindow.h \
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
    App/ExportWizard.cpp \
    App/FotowallFile.cpp \
    App/Hardware3DTest.cpp \
    App/HelpAppliance.cpp \
    App/HelpItem.cpp \
    App/HomeAppliance.cpp \
    App/HomeScene.cpp \
    App/MainWindow.cpp \
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
    App/ExportWizard.ui \
    App/HelpAppliance.ui \
    App/MainWindow.ui \
    App/PictureSearchWidget.ui \
    App/VersionCheckDialog.ui

!contains(DEFINES, NO_WORDCLOUD_APPLIANCE) {
    HEADERS += \
        App/WordcloudAppliance.h \
        App/WordcloudSidebar.h

    SOURCES += \
        App/WordcloudAppliance.cpp \
        App/WordcloudSidebar.cpp

    FORMS += \
        App/WordcloudAppliance.ui \
        App/WordcloudSidebar.ui
}
