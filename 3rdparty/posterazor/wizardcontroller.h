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

#ifndef WIZARDCONTROLLER_H
#define WIZARDCONTROLLER_H

#include <QObject>

class WizardController: public QObject
{
    Q_OBJECT
    Q_ENUMS(WizardSteps)

public:
    enum WizardSteps {
        WizardStepInputImage,
        WizardStepPaperSize,
        WizardStepOverlapping,
        WizardStepPosterSize,
        WizardStepSavePoster
    };

    WizardController(QObject *wizardDialog, QObject *parent = 0);

    void setStepAvailable(WizardSteps step, bool available);
    WizardSteps previousAvailableStep();
    WizardSteps nextAvailableStep();

public slots:
    void showManual();
    void showHelpForCurrentStep();
    void updateDialogWizardStepDescription();
    void handleImageLoaded();

private slots:
    void updateDialogWizardStep();
    void handlePrevButtonPressed();
    void handleNextButtonPressed();

signals:
    void wizardStepChanged(int step) const;
    void wizardStepDescriptionChanged(const QString &number, const QString &description) const;
    void previewStateChanged(const QString &state) const;
    void prevButtonEnabled(bool enabled) const;
    void nextButtonEnabled(bool enabled) const;
    void showManualSignal(const QString &title, const QString &manual) const;
    void showWizardStepHelpSignal(const QString &title, const QString &manual) const;

private:
    WizardSteps m_wizardStep;
    bool m_imageWasLoaded;
    QList<WizardSteps> m_unavaliableSteps;

    QString stepXofYString(WizardSteps step) const;
    static QString stepTitle(WizardSteps step);
    static QString stepHelp(WizardSteps step);
};

#endif // WIZARDCONTROLLER_H
