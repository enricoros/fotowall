/********************************************************************************
** Form generated from reading ui file 'FotoWall.ui'
**
** Created: Sat Aug 2 20:27:03 2008
**      by: Qt User Interface Compiler version 4.4.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_FOTOWALL_H
#define UI_FOTOWALL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>


class Ui_MainWindow
{
public:
    QAction *actionBlack_and_White;
    QAction *actionInvert_colors;
    QAction *actionLoad;
    QAction *actionSave;
    QAction *actionQuit;
    QAction *actionHorizontal_flip_H;
    QAction *actionVertical_flip;
    QAction *actionHorizontal_flip;
    QAction *actionRotate_90_left;
    QAction *actionRotate_90_right;
    QAction *actionSet_as_background;
    QAction *actionDocumentation;
    QAction *actionNVG;
    QAction *actionLuminosity;
    QWidget *centralwidget;
    QFrame *cmdFrame;
    QHBoxLayout *hboxLayout;
    QLabel *label;
    QLineEdit *titleEdit;
    QSpacerItem *spacerItem;
    QCheckBox *pictureBorderCheckbox;
    QPushButton *pictureBackgroundButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *addPictures;
    QPushButton *pngButton;
    QPushButton *quitButton;
    QWidget *Widget;
    QMenuBar *menubar;
    QMenu *menuFichier;
    QMenu *menuImage_effects;
    QMenu *menuImage;
    QMenu *menuHelp;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(952, 590);
    actionBlack_and_White = new QAction(MainWindow);
    actionBlack_and_White->setObjectName(QString::fromUtf8("actionBlack_and_White"));
    actionInvert_colors = new QAction(MainWindow);
    actionInvert_colors->setObjectName(QString::fromUtf8("actionInvert_colors"));
    actionLoad = new QAction(MainWindow);
    actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
    actionSave = new QAction(MainWindow);
    actionSave->setObjectName(QString::fromUtf8("actionSave"));
    actionQuit = new QAction(MainWindow);
    actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
    actionHorizontal_flip_H = new QAction(MainWindow);
    actionHorizontal_flip_H->setObjectName(QString::fromUtf8("actionHorizontal_flip_H"));
    actionVertical_flip = new QAction(MainWindow);
    actionVertical_flip->setObjectName(QString::fromUtf8("actionVertical_flip"));
    actionHorizontal_flip = new QAction(MainWindow);
    actionHorizontal_flip->setObjectName(QString::fromUtf8("actionHorizontal_flip"));
    actionRotate_90_left = new QAction(MainWindow);
    actionRotate_90_left->setObjectName(QString::fromUtf8("actionRotate_90_left"));
    const QIcon icon = QIcon(QString::fromUtf8("data/rotate_left.png"));
    actionRotate_90_left->setIcon(icon);
    actionRotate_90_right = new QAction(MainWindow);
    actionRotate_90_right->setObjectName(QString::fromUtf8("actionRotate_90_right"));
    const QIcon icon1 = QIcon(QString::fromUtf8("data/rotate_right.png"));
    actionRotate_90_right->setIcon(icon1);
    actionSet_as_background = new QAction(MainWindow);
    actionSet_as_background->setObjectName(QString::fromUtf8("actionSet_as_background"));
    actionDocumentation = new QAction(MainWindow);
    actionDocumentation->setObjectName(QString::fromUtf8("actionDocumentation"));
    actionNVG = new QAction(MainWindow);
    actionNVG->setObjectName(QString::fromUtf8("actionNVG"));
    actionLuminosity = new QAction(MainWindow);
    actionLuminosity->setObjectName(QString::fromUtf8("actionLuminosity"));
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    cmdFrame = new QFrame(centralwidget);
    cmdFrame->setObjectName(QString::fromUtf8("cmdFrame"));
    cmdFrame->setGeometry(QRect(0, 0, 951, 46));
    cmdFrame->setFrameShape(QFrame::NoFrame);
    hboxLayout = new QHBoxLayout(cmdFrame);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    label = new QLabel(cmdFrame);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout->addWidget(label);

    titleEdit = new QLineEdit(cmdFrame);
    titleEdit->setObjectName(QString::fromUtf8("titleEdit"));
    QPalette palette;
    QBrush brush(QColor(255, 255, 255, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::Base, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
    titleEdit->setPalette(palette);

    hboxLayout->addWidget(titleEdit);

    spacerItem = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    pictureBorderCheckbox = new QCheckBox(cmdFrame);
    pictureBorderCheckbox->setObjectName(QString::fromUtf8("pictureBorderCheckbox"));
    pictureBorderCheckbox->setChecked(true);

    hboxLayout->addWidget(pictureBorderCheckbox);

    pictureBackgroundButton = new QPushButton(cmdFrame);
    pictureBackgroundButton->setObjectName(QString::fromUtf8("pictureBackgroundButton"));

    hboxLayout->addWidget(pictureBackgroundButton);

    loadButton = new QPushButton(cmdFrame);
    loadButton->setObjectName(QString::fromUtf8("loadButton"));
    QPalette palette1;
    QBrush brush1(QColor(0, 0, 0, 255));
    brush1.setStyle(Qt::SolidPattern);
    palette1.setBrush(QPalette::Active, QPalette::WindowText, brush1);
    QBrush brush2(QColor(0, 0, 0, 0));
    brush2.setStyle(Qt::SolidPattern);
    palette1.setBrush(QPalette::Active, QPalette::Button, brush2);
    palette1.setBrush(QPalette::Active, QPalette::Text, brush1);
    palette1.setBrush(QPalette::Active, QPalette::ButtonText, brush1);
    palette1.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
    palette1.setBrush(QPalette::Inactive, QPalette::Button, brush2);
    palette1.setBrush(QPalette::Inactive, QPalette::Text, brush1);
    palette1.setBrush(QPalette::Inactive, QPalette::ButtonText, brush1);
    palette1.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
    palette1.setBrush(QPalette::Disabled, QPalette::Button, brush2);
    palette1.setBrush(QPalette::Disabled, QPalette::Text, brush1);
    palette1.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
    loadButton->setPalette(palette1);

    hboxLayout->addWidget(loadButton);

    saveButton = new QPushButton(cmdFrame);
    saveButton->setObjectName(QString::fromUtf8("saveButton"));
    QPalette palette2;
    palette2.setBrush(QPalette::Active, QPalette::WindowText, brush1);
    palette2.setBrush(QPalette::Active, QPalette::Button, brush2);
    palette2.setBrush(QPalette::Active, QPalette::Text, brush1);
    palette2.setBrush(QPalette::Active, QPalette::ButtonText, brush1);
    palette2.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
    palette2.setBrush(QPalette::Inactive, QPalette::Button, brush2);
    palette2.setBrush(QPalette::Inactive, QPalette::Text, brush1);
    palette2.setBrush(QPalette::Inactive, QPalette::ButtonText, brush1);
    palette2.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
    palette2.setBrush(QPalette::Disabled, QPalette::Button, brush2);
    palette2.setBrush(QPalette::Disabled, QPalette::Text, brush1);
    palette2.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
    saveButton->setPalette(palette2);

    hboxLayout->addWidget(saveButton);

    addPictures = new QPushButton(cmdFrame);
    addPictures->setObjectName(QString::fromUtf8("addPictures"));

    hboxLayout->addWidget(addPictures);

    pngButton = new QPushButton(cmdFrame);
    pngButton->setObjectName(QString::fromUtf8("pngButton"));
    QPalette palette3;
    palette3.setBrush(QPalette::Active, QPalette::WindowText, brush1);
    palette3.setBrush(QPalette::Active, QPalette::Button, brush2);
    palette3.setBrush(QPalette::Active, QPalette::Text, brush1);
    palette3.setBrush(QPalette::Active, QPalette::ButtonText, brush1);
    palette3.setBrush(QPalette::Inactive, QPalette::WindowText, brush1);
    palette3.setBrush(QPalette::Inactive, QPalette::Button, brush2);
    palette3.setBrush(QPalette::Inactive, QPalette::Text, brush1);
    palette3.setBrush(QPalette::Inactive, QPalette::ButtonText, brush1);
    palette3.setBrush(QPalette::Disabled, QPalette::WindowText, brush1);
    palette3.setBrush(QPalette::Disabled, QPalette::Button, brush2);
    palette3.setBrush(QPalette::Disabled, QPalette::Text, brush1);
    palette3.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
    pngButton->setPalette(palette3);

    hboxLayout->addWidget(pngButton);

    quitButton = new QPushButton(cmdFrame);
    quitButton->setObjectName(QString::fromUtf8("quitButton"));

    hboxLayout->addWidget(quitButton);

    Widget = new QWidget(centralwidget);
    Widget->setObjectName(QString::fromUtf8("Widget"));
    Widget->setGeometry(QRect(0, 40, 951, 511));
    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 952, 27));
    menuFichier = new QMenu(menubar);
    menuFichier->setObjectName(QString::fromUtf8("menuFichier"));
    menuImage_effects = new QMenu(menubar);
    menuImage_effects->setObjectName(QString::fromUtf8("menuImage_effects"));
    menuImage = new QMenu(menubar);
    menuImage->setObjectName(QString::fromUtf8("menuImage"));
    menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);
    label->setBuddy(titleEdit);

    menubar->addAction(menuFichier->menuAction());
    menubar->addAction(menuImage->menuAction());
    menubar->addAction(menuImage_effects->menuAction());
    menubar->addAction(menuHelp->menuAction());
    menuFichier->addAction(actionLoad);
    menuFichier->addAction(actionSave);
    menuFichier->addAction(actionQuit);
    menuImage_effects->addAction(actionBlack_and_White);
    menuImage_effects->addAction(actionNVG);
    menuImage_effects->addAction(actionInvert_colors);
    menuImage_effects->addAction(actionHorizontal_flip);
    menuImage_effects->addAction(actionVertical_flip);
    menuImage_effects->addAction(actionLuminosity);
    menuImage->addAction(actionRotate_90_left);
    menuImage->addAction(actionRotate_90_right);
    menuImage->addAction(actionSet_as_background);
    menuHelp->addAction(actionDocumentation);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
    actionBlack_and_White->setText(QApplication::translate("MainWindow", "Black and White (W)", 0, QApplication::UnicodeUTF8));
    actionInvert_colors->setText(QApplication::translate("MainWindow", "Invert colors         (I)", 0, QApplication::UnicodeUTF8));
    actionLoad->setText(QApplication::translate("MainWindow", "Load...", 0, QApplication::UnicodeUTF8));
    actionSave->setText(QApplication::translate("MainWindow", "Save...", 0, QApplication::UnicodeUTF8));
    actionQuit->setText(QApplication::translate("MainWindow", "Quit", 0, QApplication::UnicodeUTF8));
    actionHorizontal_flip_H->setText(QApplication::translate("MainWindow", "Horizontal flip (H)", 0, QApplication::UnicodeUTF8));
    actionVertical_flip->setText(QApplication::translate("MainWindow", "Vertical flip           (V)", 0, QApplication::UnicodeUTF8));
    actionHorizontal_flip->setText(QApplication::translate("MainWindow", "Horizontal flip      (H)", 0, QApplication::UnicodeUTF8));
    actionRotate_90_left->setText(QApplication::translate("MainWindow", "Rotate 90\302\260 left       (L)", 0, QApplication::UnicodeUTF8));
    actionRotate_90_right->setText(QApplication::translate("MainWindow", "Rotate 90\302\260 right    (R)", 0, QApplication::UnicodeUTF8));
    actionSet_as_background->setText(QApplication::translate("MainWindow", "Set as background", 0, QApplication::UnicodeUTF8));
    actionDocumentation->setText(QApplication::translate("MainWindow", "Documentation", 0, QApplication::UnicodeUTF8));
    actionNVG->setText(QApplication::translate("MainWindow", "Level of gray (NVG)  (B)", 0, QApplication::UnicodeUTF8));
    actionLuminosity->setText(QApplication::translate("MainWindow", "Luminosity", 0, QApplication::UnicodeUTF8));
    cmdFrame->setStyleSheet(QApplication::translate("MainWindow", "QFrame {\n"
"    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(255, 255, 255, 255), stop:0.461 rgba(232, 232, 232, 255), stop:0.463 rgba(202, 202, 202, 255), stop:1 rgba(237, 237, 237, 255))\n"
"}", 0, QApplication::UnicodeUTF8));
    label->setStyleSheet(QApplication::translate("MainWindow", "color: black\n"
"", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MainWindow", "Title", 0, QApplication::UnicodeUTF8));
    titleEdit->setStyleSheet(QApplication::translate("MainWindow", "background-color: white", 0, QApplication::UnicodeUTF8));
    titleEdit->setText(QString());
    pictureBorderCheckbox->setStyleSheet(QApplication::translate("MainWindow", "color: black;", 0, QApplication::UnicodeUTF8));
    pictureBorderCheckbox->setText(QApplication::translate("MainWindow", "Picture border", 0, QApplication::UnicodeUTF8));
    pictureBackgroundButton->setStyleSheet(QApplication::translate("MainWindow", "background-color : transparent; color:black", 0, QApplication::UnicodeUTF8));
    pictureBackgroundButton->setText(QApplication::translate("MainWindow", "Picture background", 0, QApplication::UnicodeUTF8));
    loadButton->setStyleSheet(QApplication::translate("MainWindow", "background-color : transparent; color:black\n"
"", 0, QApplication::UnicodeUTF8));
    loadButton->setText(QApplication::translate("MainWindow", "Load...", 0, QApplication::UnicodeUTF8));
    saveButton->setStyleSheet(QApplication::translate("MainWindow", "background-color: transparent;\n"
"color:black;", 0, QApplication::UnicodeUTF8));
    saveButton->setText(QApplication::translate("MainWindow", "Save...", 0, QApplication::UnicodeUTF8));
    addPictures->setStyleSheet(QApplication::translate("MainWindow", "background-color: transparent;\n"
"color:black;", 0, QApplication::UnicodeUTF8));
    addPictures->setText(QApplication::translate("MainWindow", "Add Pictures", 0, QApplication::UnicodeUTF8));
    pngButton->setStyleSheet(QApplication::translate("MainWindow", "background-color: transparent;\n"
"color:black;", 0, QApplication::UnicodeUTF8));
    pngButton->setText(QApplication::translate("MainWindow", "Render PNG", 0, QApplication::UnicodeUTF8));
    quitButton->setStyleSheet(QApplication::translate("MainWindow", "background-color: transparent;\n"
"color:black;", 0, QApplication::UnicodeUTF8));
    quitButton->setText(QApplication::translate("MainWindow", "Quit", 0, QApplication::UnicodeUTF8));
    Widget->setStyleSheet(QApplication::translate("MainWindow", "background-color: transparent;\n"
"color:black;", 0, QApplication::UnicodeUTF8));
    menuFichier->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    menuImage_effects->setTitle(QApplication::translate("MainWindow", "Image effects", 0, QApplication::UnicodeUTF8));
    menuImage->setTitle(QApplication::translate("MainWindow", "Image", 0, QApplication::UnicodeUTF8));
    menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FOTOWALL_H
