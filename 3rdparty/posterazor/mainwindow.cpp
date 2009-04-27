/*
    PosteRazor - Make your own poster!
    Copyright (C) 2005-2009 by Alessandro Portale
    http://posterazor.sourceforge.net/

    This file is part of PosteRazor

    PosteRazor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PosteRazor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PosteRazor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "mainwindow.h"
#include <QSignalMapper>
#include <QFileDialog>
#include <QTranslator>
#include <QtDebug>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QMessageBox>
#include <QSettings>
#include <QMetaMethod>

const QLatin1String settingsKey_MainWindowGeometry("MainWindowGeometry");

static QString applicationNameWithVersion()
{
    return
        QCoreApplication::applicationName()
#if QT_VERSION >= 0x040400
        + QLatin1Char(' ') + QCoreApplication::applicationVersion()
#endif
    ;
}

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    setupUi(this);

    m_unitOfLengthActions = new QActionGroup(m_menuSettings);

    m_actionLoadInputImage->setShortcuts(
        QList<QKeySequence>() << (Qt::CTRL + Qt::Key_L) << (Qt::CTRL + Qt::Key_O));
    m_actionSavePoster->setShortcut(Qt::CTRL + Qt::Key_S);
    m_actionExit->setShortcut(Qt::CTRL + Qt::Key_Q);
    m_actionPosteRazorManual->setShortcut(Qt::Key_F1);
    m_actionLoadInputImage->setIcon(QApplication::style()->standardPixmap(QStyle::SP_DirOpenIcon));
    m_actionSavePoster->setIcon(QApplication::style()->standardPixmap(QStyle::SP_DialogSaveButton));
    m_actionSavePoster->setEnabled(false);

    setWindowIcon(QIcon(":/Icons/posterazor.png"));

    setWindowTitle(applicationNameWithVersion());
    createConnections();
    populateUI();
    retranslateUi();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    QMainWindow::changeEvent(event);
}

void MainWindow::retranslateUi()
{
    // TODO: Add shortcut and trailing "..." for load and save action. Enhance cleanString and use it in WizardController
    m_actionLoadInputImage->setText(                QCoreApplication::translate("Help", "Load an input image"));
    m_actionSavePoster->setText(                    QCoreApplication::translate("Help", "Save the Poster"));
    m_actionPosteRazorManual->setText(              QCoreApplication::translate("Help", "&Manual"));
    m_actionPosteRazorWebSite->setText(             QCoreApplication::translate("Help", "PosteRazor &website"));
    m_actionAboutPosteRazor->setText(               QCoreApplication::translate("Help", "&About PosteRazor"));
    m_actionAboutQt->setText(                       QCoreApplication::translate("Help", "About &Qt"));
    m_actionExit->setText(                          QCoreApplication::translate("Main window", "E&xit"));
    m_menuFile->setTitle(                           QCoreApplication::translate("Main window", "&File"));
    m_menuSettings->setTitle(                       QCoreApplication::translate("Main window", "&Settings"));
}

void MainWindow::setPaperFormat(const QString &format)
{
    m_wizard->setPaperFormat(format);
}

void MainWindow::setPaperOrientation(QPrinter::Orientation orientation)
{
    m_wizard->setPaperOrientation(orientation);
}

void MainWindow::setPaperBorderTop(double border)
{
    m_wizard->setPaperBorderTop(border);
}

void MainWindow::setPaperBorderRight(double border)
{
    m_wizard->setPaperBorderRight(border);
}

void MainWindow::setPaperBorderBottom(double border)
{
    m_wizard->setPaperBorderBottom(border);
}

void MainWindow::setPaperBorderLeft(double border)
{
    m_wizard->setPaperBorderLeft(border);
}

void MainWindow::setCustomPaperSize(const QSizeF &size)
{
    m_wizard->setCustomPaperSize(size);
}

void MainWindow::setUseCustomPaperSize(bool useIt)
{
    m_wizard->setUseCustomPaperSize(useIt);
}

void MainWindow::setOverlappingWidth(double width)
{
    m_wizard->setOverlappingWidth(width);
}

void MainWindow::setOverlappingHeight(double height)
{
    m_wizard->setOverlappingHeight(height);
}

void MainWindow::setOverlappingPosition(Qt::Alignment position)
{
    m_wizard->setOverlappingPosition(position);
}

void MainWindow::setPosterWidthAbsolute(double width)
{
    m_wizard->setPosterWidthAbsolute(width);
}

void MainWindow::setPosterHeightAbsolute(double height)
{
    m_wizard->setPosterHeightAbsolute(height);
}

void MainWindow::setPosterWidthPages(double width)
{
    m_wizard->setPosterWidthPages(width);
}

void MainWindow::setPosterHeightPages(double height)
{
    m_wizard->setPosterHeightPages(height);
}

void MainWindow::setPosterSizePercentual(double percent)
{
    m_wizard->setPosterSizePercentual(percent);
}

void MainWindow::setPosterSizeMode(Types::PosterSizeModes mode)
{
    m_wizard->setPosterSizeMode(mode);
}

void MainWindow::setPosterAlignment(Qt::Alignment alignment)
{
    m_wizard->setPosterAlignment(alignment);
}

void MainWindow::setLaunchPDFApplication(bool launch)
{
    m_wizard->setLaunchPDFApplication(launch);
}

void MainWindow::updatePreview()
{
    m_wizard->updatePreview();
}

void MainWindow::showImageFileName(const QString &fileName)
{
    m_wizard->showImageFileName(fileName);
}

void MainWindow::updateImageInfoFields(const QSize &inputImageSizeInPixels, const QSizeF &imageSize, double verticalDpi, double horizontalDpi, Types::ColorTypes colorType, int bitsPerPixel)
{
    m_wizard->updateImageInfoFields(inputImageSizeInPixels, imageSize, verticalDpi, horizontalDpi, colorType, bitsPerPixel);
}

void MainWindow::setCurrentTranslation(const QString &translation)
{
    QAction *translationAction = NULL;
    translationAction = m_translationActions.value(translation);
    if (!translationAction) {
        // On a Swiss system 'translation' may be "de_CH". So let's fall back to "de"
        const QString translationLanguage = translation.split(QLatin1Char('_')).first();
        translationAction = m_translationActions.value(translationLanguage);
    }
    if (translationAction)
        translationAction->setChecked(true);
}

void MainWindow::setCurrentUnitOfLength(const QString &unit)
{
    foreach (QAction *action, m_unitOfLengthActions->actions()) {
        if (action->text() == unit) {
            action->setChecked(true);
            break;
        }
    }
    m_wizard->setCurrentUnitOfLength(unit);
}

void MainWindow::addAboutDialogAction(QAction *action)
{
    m_menuAbout->addAction(action);
}

void MainWindow::setPrevButtonEnabled(bool enabled)
{
    m_wizard->setPrevButtonEnabled(enabled);
}

void MainWindow::setNextButtonEnabled(bool enabled)
{
    m_wizard->setNextButtonEnabled(enabled);
}

void MainWindow::setPosterSavingEnabled(bool enabled)
{
    m_actionSavePoster->setEnabled(enabled);
    m_wizard->setPosterSavingEnabled(enabled);
}

void MainWindow::setWizardStep(int step)
{
    m_wizard->setWizardStep(step);
}

void MainWindow::setWizardStepDescription(const QString &number, const QString &description)
{
    m_wizard->setWizardStepDescription(number, description);
}

void MainWindow::setPreviewState(const QString &state)
{
    m_wizard->setPreviewState(state);
}

void MainWindow::setPreviewImage(const QImage &image)
{
    m_wizard->setPreviewImage(image);
}

void MainWindow::showWizardStepHelp(const QString &title, const QString &text)
{
    QMessageBox box(this);
    box.setWindowTitle(title);
    QString helpText = text;
#if defined(Q_WS_MAC)
    // Hack. Since QMessageBoxPrivate sets the whole font to bold on Q_WS_MAC (no matter which style),
    // we put emphasis on the key words by setting them to italic and into single quotes.
    helpText.replace("<b>", "<i>'");
    helpText.replace("</b>", "'</i>");
#endif
    box.setText(helpText);
    box.setTextFormat(Qt::RichText);
    box.addButton(QMessageBox::Ok);
    box.exec();
}

void MainWindow::showManual(const QString &title, const QString &text)
{
    QDialog *dialog = new QDialog(this);
    dialog->setModal(true);
    dialog->setWindowTitle(title);
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->setWindowFlags(dialog->windowFlags() ^ Qt::WindowContextHelpButtonHint);
    dialog->resize(500, 400);
    dialog->setLayout(new QVBoxLayout);
    QTextBrowser *browser = new QTextBrowser;
    browser->setOpenExternalLinks(true);
    browser->document()->setDefaultStyleSheet(
        "dt {font-weight: bold;}"
        "dd {margin-left: 15px;}"
        "p {text-indent: 7px;}"
        "p, dd {margin-top: 0px; margin-bottom: 6px;}"
        "h2 {margin-top: 18px; margin-bottom: 6px;}");
    browser->setHtml(text);
    dialog->layout()->addWidget(browser);
    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted ()), dialog, SLOT(accept()));
    dialog->layout()->addWidget(buttonBox);
    dialog->show();
}

void MainWindow::handleTranslationAction(QAction *action) const
{
    emit translationChanged(action->data().toString());
}

void MainWindow::handleUnitOfLengthAction(QAction *action) const
{
    emit unitOfLengthChanged(action->text());
}

void MainWindow::createConnections()
{
    connect(m_actionExit,                           SIGNAL(triggered()),                SLOT(close()));
    connect(m_actionLoadInputImage,                 SIGNAL(triggered()),                SIGNAL(loadImageSignal()));
    connect(m_actionSavePoster,                     SIGNAL(triggered()),                SIGNAL(savePosterSignal()));
    connect(m_actionPosteRazorWebSite,              SIGNAL(triggered()),                SIGNAL(openPosteRazorWebsiteSignal()));
    connect(m_actionAboutQt,                        SIGNAL(triggered()),                SLOT(showAboutQtDialog()));
    connect(m_actionAboutPosteRazor,                SIGNAL(triggered()),                SLOT(showAboutPosteRazorDialog()));
    connect(m_actionPosteRazorManual,               SIGNAL(triggered()),                SIGNAL(manualSignal()));

    static const char* const relayedSignals[] = {
        SIGNAL(paperFormatChanged(const QString&)),
        SIGNAL(useCustomPaperSizeChanged(bool)),
        SIGNAL(paperOrientationChanged(QPrinter::Orientation)),
        SIGNAL(paperCustomWidthChanged(double)),
        SIGNAL(paperCustomHeightChanged(double)),
        SIGNAL(paperBorderTopChanged(double)),
        SIGNAL(paperBorderRightChanged(double)),
        SIGNAL(paperBorderBottomChanged(double)),
        SIGNAL(paperBorderLeftChanged(double)),
        SIGNAL(overlappingWidthChanged(double)),
        SIGNAL(overlappingHeightChanged(double)),
        SIGNAL(overlappingPositionChanged(Qt::Alignment)),
        SIGNAL(posterWidthAbsoluteChanged(double)),
        SIGNAL(posterHeightAbsoluteChanged(double)),
        SIGNAL(posterWidthPagesChanged(double)),
        SIGNAL(posterHeightPagesChanged(double)),
        SIGNAL(posterSizePercentualChanged(double)),
        SIGNAL(posterAlignmentChanged(Qt::Alignment)),
        SIGNAL(savePosterSelected()),
        SIGNAL(launchPDFApplicationChanged(bool)),
        SIGNAL(nextButtonPressed()),
        SIGNAL(prevButtonPressed()),
        SIGNAL(wizardStepHelpSignal()),
        SIGNAL(savePosterSignal()),
        SIGNAL(loadImageSignal()),
        SIGNAL(needsPaint(PaintCanvasInterface*, const QVariant&)),
        SIGNAL(imageLoaded()),
    };
    static const int relayedSignalsCount = sizeof(relayedSignals) / sizeof(relayedSignals[0]);
    for (int i = 0; i < relayedSignalsCount; ++i)
        connect(m_wizard, relayedSignals[i], relayedSignals[i]);
}

void MainWindow::populateUI()
{
    connect (m_unitOfLengthActions, SIGNAL(triggered(QAction*)), SLOT(handleUnitOfLengthAction(QAction*)));
    m_unitOfLengthActions->setExclusive(true);
    const int unitsOfLengthCount = Types::unitsOfLength().count();
    for (int i = 0; i < unitsOfLengthCount; i++) {
        const QString unitOfLength = Types::unitsOfLength().value((Types::UnitsOfLength)i).first;
        QAction *unitOfLengthAction = m_unitOfLengthActions->addAction(unitOfLength);
        unitOfLengthAction->setCheckable(true);
    }
    m_menuSettings->addActions(m_unitOfLengthActions->actions());

    m_menuSettings->addSeparator();

    QActionGroup *translationActions = new QActionGroup(m_menuSettings);
    connect (translationActions, SIGNAL(triggered(QAction*)), SLOT(handleTranslationAction(QAction*)));
    translationActions->setExclusive(true);
    QDir translationDir(":/Translations/");
    foreach (const QFileInfo &translation, translationDir.entryInfoList(QDir::Files)) {
        QTranslator translator;
        translator.load(translation.absoluteFilePath());
        QAction *languageAction = translationActions->addAction(translator.translate("Main window", "Language name"));
        // TODO: Find out why the menu entries are empty on Mac (and maybe on Linux?)
        const QString localeString(translation.baseName());
        languageAction->setData(localeString);
        languageAction->setCheckable(true);
        m_translationActions.insert(localeString, languageAction);
    }
    m_menuSettings->addActions(translationActions->actions());
}

void MainWindow::showAboutQtDialog() const
{
    QApplication::aboutQt();
}

void MainWindow::showAboutPosteRazorDialog()
{
    const QString title = Types::cleanString(QCoreApplication::translate("Help", "&About PosteRazor"));
    const QString webpageAnchor = QString(QLatin1String("<a href=\"%1\">posterazor.sourceforge.net</a>"))
        .arg(QCoreApplication::translate("Help", "http://posterazor.sourceforge.net/", "Only translate, if the website has this language."));
    const QString aboutText =
        QLatin1String("<h1>") + applicationNameWithVersion() +
        QLatin1String("</h1>") +
        Types::newlineToParagraph(QCoreApplication::translate("Help",
            "The PosteRazor cuts a raster image into pieces which can be printed on a printer and be put together to a poster.\n"
            "As an input image, raster files of various image file formats are supported. Instead of directly printing the poster, the PosteRazor produces a multi page PDF file which contains the poster tiles.\n"
            "It is an open source program which depends on other open source projects. The PosteRazor is hosted on %1.")
            .arg(webpageAnchor)) +
        QLatin1String("<h2>Code from third parties</h2>"
            "<dl>"
            "<dt>Qt</dt>"
            "<dd>Extensive use of <a href=\"http://www.trolltech.com/\">Trolltech</a>s amazing cross-platform application framework</dd>"
            "<dt>FreeImage</dt>"
            "<dd>Easy and powerful image format support. See the <a href=\"http://freeimage.sourceforge.net/\">FreeImage website</a></dd>"
            "</dl>") +
        QLatin1String("<h2>Translations</h2>"
            "<p>In order of appearance</p>"
            "<dl>"
            "<dt>English</dt><dd>Alessandro Portale (author)</dd>"
            "<dt>German</dt><dd>Alessandro Portale (author)</dd>"
            "<dt>Polish</dt><dd>Grzegorz Wacikowski</dd>"
            "<dt>French</dt><dd>Martin Loyer</dd>"
            "<dt>Italian</dt><dd>Stefano Nosei</dd>"
            "<dt>Dutch/Belgian</dt><dd>Erik Wijkamp</dd>"
            "<dt>Spanish</dt><dd><a href=\"http://hispanicoweb.net/\">Hispánico Web</a></dd>"
            "<dt>Brazilian Portuguese</dt><dd>Éderson Gomes dos Santos</dd>"
            "<dt>European Portuguese</dt><dd>Helder Correia</dd>"
            "<dt>Simplified Chinese</dt><dd>Sun Li</dd>"
            "<dt>Finnish</dt><dd>Olli</dd>"
            "</dl>") +
        QLatin1String("<h2>License</h2>"
            "<p>PosteRazor - Make your own poster!<br/>"
            "posterazor.sourceforge.net<br/>"
            "Copyright (C) 2005-2009 by Alessandro Portale</p>"
            "<p>This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.</p>"
            "<p>This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p>"
            "<p>You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.</p>");
    showManual(title, aboutText);
}

void MainWindow::readSettings(const QSettings *settings)
{
    restoreGeometry(settings->value(settingsKey_MainWindowGeometry, QByteArray()).toByteArray());
}

void MainWindow::writeSettings(QSettings *settings) const
{
    settings->setValue(settingsKey_MainWindowGeometry, saveGeometry());
}
