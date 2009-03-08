/********************************************************************************
** Form generated from reading ui file 'BackgroundChooser.ui'
**
** Created: Sat Aug 2 20:27:03 2008
**      by: Qt User Interface Compiler version 4.4.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_BACKGROUNDCHOOSER_H
#define UI_BACKGROUNDCHOOSER_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_BackgroundChooser
{
public:
    QDialogButtonBox *buttonBox;
    QListWidget *listWidget;
    QPushButton *imageButton;

    void setupUi(QDialog *BackgroundChooser)
    {
    if (BackgroundChooser->objectName().isEmpty())
        BackgroundChooser->setObjectName(QString::fromUtf8("BackgroundChooser"));
    BackgroundChooser->resize(393, 326);
    buttonBox = new QDialogButtonBox(BackgroundChooser);
    buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
    buttonBox->setGeometry(QRect(30, 290, 341, 32));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::NoButton|QDialogButtonBox::Ok);
    listWidget = new QListWidget(BackgroundChooser);
    listWidget->setObjectName(QString::fromUtf8("listWidget"));
    listWidget->setGeometry(QRect(0, 0, 391, 251));
    imageButton = new QPushButton(BackgroundChooser);
    imageButton->setObjectName(QString::fromUtf8("imageButton"));
    imageButton->setGeometry(QRect(130, 250, 173, 30));

    retranslateUi(BackgroundChooser);
    QObject::connect(buttonBox, SIGNAL(accepted()), BackgroundChooser, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), BackgroundChooser, SLOT(reject()));

    QMetaObject::connectSlotsByName(BackgroundChooser);
    } // setupUi

    void retranslateUi(QDialog *BackgroundChooser)
    {
    BackgroundChooser->setWindowTitle(QApplication::translate("BackgroundChooser", "Background Chooser", 0, QApplication::UnicodeUTF8));
    imageButton->setText(QApplication::translate("BackgroundChooser", "Add other image...", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(BackgroundChooser);
    } // retranslateUi

};

namespace Ui {
    class BackgroundChooser: public Ui_BackgroundChooser {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BACKGROUNDCHOOSER_H
