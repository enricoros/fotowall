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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "posterazorcore.h"

class QActionGroup;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);

    void retranslateUi();
    void retranslateUiWithDimensionUnit();
    void changeEvent(QEvent *event);

public slots:
    void setPaperFormat(const QString &format);
    void setPaperOrientation(QPrinter::Orientation orientation);
    void setPaperBorderTop(double border);
    void setPaperBorderRight(double border);
    void setPaperBorderBottom(double border);
    void setPaperBorderLeft(double border);
    void setCustomPaperSize(const QSizeF &size);
    void setUseCustomPaperSize(bool useIt);
    void setOverlappingWidth(double width);
    void setOverlappingHeight(double height);
    void setOverlappingPosition(Qt::Alignment position);
    void setPosterWidthAbsolute(double width);
    void setPosterHeightAbsolute(double height);
    void setPosterWidthPages(double width);
    void setPosterHeightPages(double height);
    void setPosterSizePercentual(double percent);
    void setPosterSizeMode(Types::PosterSizeModes mode);
    void setPosterAlignment(Qt::Alignment alignment);
    void setLaunchPDFApplication(bool launch);
    void updatePreview();
    void showImageFileName(const QString &fileName);
    void updateImageInfoFields(const QSize &inputImageSizeInPixels, const QSizeF &imageSize, double verticalDpi, double horizontalDpi, Types::ColorTypes colorType, int bitsPerPixel);
    void setCurrentTranslation(const QString &translation); // Only to set the right menu entry to checked
    void setCurrentUnitOfLength(const QString &unit);
    void addAboutDialogAction(QAction *action);
    void readSettings(const QSettings *settings);
    void writeSettings(QSettings *settings) const;
    void setPreviewImage(const QImage &image);
    void setPrevButtonEnabled(bool enabled);
    void setNextButtonEnabled(bool enabled);
    void setPosterSavingEnabled(bool enabled);
    void setWizardStep(int step);
    void setWizardStepDescription(const QString &number, const QString &description);
    void setPreviewState(const QString &state);
    void showWizardStepHelp(const QString &title, const QString &text);
    void showManual(const QString &title, const QString &text);

private:
    PosteRazorCore *m_posteRazor;
    QString m_currentUnitOfLength;
    QActionGroup *m_unitOfLengthActions;
    QHash<QString, QAction*> m_translationActions;

    void createConnections();
    void populateUI();

signals:
    void paperFormatChanged(const QString &format) const;
    void useCustomPaperSizeChanged(bool useIt) const;
    void paperOrientationChanged(QPrinter::Orientation orientation) const;
    void paperCustomWidthChanged(double width) const;
    void paperCustomHeightChanged(double height) const;
    void paperBorderTopChanged(double border) const;
    void paperBorderRightChanged(double border) const;
    void paperBorderBottomChanged(double border) const;
    void paperBorderLeftChanged(double border) const;
    void overlappingWidthChanged(double width) const;
    void overlappingHeightChanged(double height) const;
    void overlappingPositionChanged(Qt::Alignment position) const;
    void posterWidthAbsoluteChanged(double width) const;
    void posterHeightAbsoluteChanged(double height) const;
    void posterWidthPagesChanged(double width) const;
    void posterHeightPagesChanged(double height) const;
    void posterSizePercentualChanged(double percent) const;
    void posterAlignmentChanged(Qt::Alignment alignment) const;
    void savePosterSelected() const;
    void launchPDFApplicationChanged(bool launch) const;
    void nextButtonPressed() const;
    void prevButtonPressed() const;
    void wizardStepHelpSignal() const;
    void manualSignal() const;
    void savePosterSignal() const;
    void loadImageSignal() const;
    void needsPaint(PaintCanvasInterface *paintDevice, const QVariant &options) const;
    void imageLoaded() const;
    void translationChanged(const QString &localeName = QString()) const;
    void unitOfLengthChanged(const QString &unit) const;
    void openPosteRazorWebsiteSignal() const;

private slots:
    void handleTranslationAction(QAction *action) const;
    void handleUnitOfLengthAction(QAction *action) const;
    void showAboutQtDialog() const;
    void showAboutPosteRazorDialog();
};

#endif // MAINWINDOW_H
