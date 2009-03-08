/********************************************************************************
** Form generated from reading ui file 'Aide.ui'
**
** Created: Sat Aug 2 20:27:03 2008
**      by: Qt User Interface Compiler version 4.4.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_AIDE_H
#define UI_AIDE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE;

class Ui_HelpDialog
{
public:
    QWidget *horizontalLayout;
    QHBoxLayout *hboxLayout;
    QLabel *label;

    void setupUi(QDialog *HelpDialog)
    {
    if (HelpDialog->objectName().isEmpty())
        HelpDialog->setObjectName(QString::fromUtf8("HelpDialog"));
    HelpDialog->setWindowModality(Qt::NonModal);
    HelpDialog->setEnabled(true);
    HelpDialog->resize(1000, 598);
    horizontalLayout = new QWidget(HelpDialog);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setGeometry(QRect(0, 0, 1001, 601));
    hboxLayout = new QHBoxLayout(horizontalLayout);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    hboxLayout->setContentsMargins(0, 0, 0, 0);
    label = new QLabel(horizontalLayout);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout->addWidget(label);


    retranslateUi(HelpDialog);

    QMetaObject::connectSlotsByName(HelpDialog);
    } // setupUi

    void retranslateUi(QDialog *HelpDialog)
    {
    HelpDialog->setWindowTitle(QApplication::translate("HelpDialog", "Aide", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("HelpDialog", "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'DejaVu Sans'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:18px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:xx-large; font-weight:600;\"><span style=\" font-size:xx-large;\">Aide</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Fotowall est un programme permettant de cr\303\251er facilement des fonds d'\303\251cran, des </p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">pr\303\251sentations d'images...</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" ma"
        "rgin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Vous pouvez:</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">- <span style=\" font-weight:600;\">d\303\251placer des images</span> gr\303\242ce \303\240 un simple cliquer glisser</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">- les <span style=\" font-weight:600;\">redimmensionner</span> en cliquant sur le bouton rouge en bas \303\240 gauche de l'image. Si vous appuyez sur la touche Ctrl en m\303\252me temps </p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">  que vous redimmensionnez, les proportions seront conserv\303\251es.</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">- les<span sty"
        "le=\" font-weight:600;\"> faire pivoter</span> en faisant un cliquer glisser sur le bouton vert en bas \303\240 droite de l'image</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Vous pouvez appliquer des transformations \303\240 l'image:</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">- <span style=\" font-weight:600;\">inversion horizontale</span> : Appuyez sur la touche \"H\"</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">-<span style=\" font-weight:600;\"> invertion verticale</span> : Appuyez sur la touche \"V\"</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-inden"
        "t:0; text-indent:0px;\">- <span style=\" font-weight:600;\">rotation</span> : Permet de tourner l'image de 90\302\260. Appuyez sur \"L\" (ou Image-&gt;Pivoter vers la gauche) pour tourner dans le sens inverse </p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">  des aiguilles d'une montre, \"R\" (ou Image-&gt;Pivoter vers la droite) pour tourner dans l'autre sens.</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Vous pouvez \303\251galement apliquer des effets \303\240 l'image:</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">- <span style=\" font-weight:600;\">Noir et blanc</span> <span style=\" font-weight:600;\">:</span> "
        "soit en s\303\251lectionnant l'image puis en appuyant sur la touche \"B\", soit en allant dans le menu Effets-&gt;Noir et blanc</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">- <span style=\" font-weight:600;\">Invertion des couleurs : </span>Appuyez sur la touche \"I\" ou allez dans le menu Effets-&gt;Inverser les couleurs</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Vous pouvez sauvegarder une image en particulier en appuyant sur la touche \"S\" apr\303\250s avoir s\303\251lectionner une image.</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" margin-top:0px; margin-bott"
        "om:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Pour <span style=\" font-weight:600;\">enregistrer votre fond d'\303\251cran</span>, cliquez sur \"Exporter en PNG\".</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Vous pouvez \303\251galement<span style=\" font-weight:600;\"> sauvegarder votre travail en cours</span> en cliquant sur \"Sauvegarder\". Vous pourrez ensuite le reprendre au m\303\252me endroit </p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">en cliquant sur \"Ouvrir\".</p></body></html>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(HelpDialog);
    } // retranslateUi

};

namespace Ui {
    class HelpDialog: public Ui_HelpDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AIDE_H
