/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ButtonsDialog.h"
#include <QAbstractButton>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

ButtonsDialog::ButtonsDialog(const QString & dialogId, QWidget * parent)
  : QDialog(parent)
  , m_dialogId(dialogId)
  , m_memorize(false)
  , m_layout(0)
  , m_messageLabel(0)
  , m_buttons(0)
  , m_defaultButton(QDialogButtonBox::NoButton)
  , m_pressedButton(QDialogButtonBox::NoButton)
{
    // create contents
    m_layout = new QVBoxLayout(this);

    m_messageLabel = new QLabel(this);
    m_layout->addWidget(m_messageLabel);

    m_buttons = new QDialogButtonBox(this);
    m_layout->addWidget(m_buttons);
    m_buttons->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    //setDefaultButton(QDialogButtonBox::Ok);
    connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(slotButtonClicked(QAbstractButton *)));
}

QDialogButtonBox::StandardButton ButtonsDialog::execute()
{
    // handle a previous answer if memorize is on
    QString configKey = "ButtonsDialog/" + m_dialogId;
    if (m_memorize) {
        // return the previous answer if such answer exists
        QSettings s;
        if (s.contains(configKey))
            return (QDialogButtonBox::StandardButton)s.value(configKey).toInt();
    }

    // pop up the dialog
    m_pressedButton = QDialogButtonBox::NoButton;
    int retCode = exec();
    if (retCode == QDialog::Rejected || m_pressedButton == QDialogButtonBox::NoButton)
        return QDialogButtonBox::NoButton;

    // save the answer if requested
    if (m_memorize)
        QSettings().setValue(configKey, (int)m_pressedButton);

    // return the choosen key
    return m_pressedButton;
}

void ButtonsDialog::setButtons(QDialogButtonBox::StandardButtons buttons)
{
    m_buttons->setStandardButtons(buttons);
    setDefaultButton(m_defaultButton);
    foreach (QDialogButtonBox::StandardButton sb, m_buttonNames.keys())
        setButtonText(sb, m_buttonNames[sb]);
}

QDialogButtonBox::StandardButtons ButtonsDialog::buttons() const
{
    return m_buttons->standardButtons();
}

void ButtonsDialog::setButtonText(QDialogButtonBox::StandardButton sb, const QString & text)
{
    m_buttonNames[sb] = text;
    if (QPushButton * pButton = m_buttons->button(sb))
        pButton->setText(text);
}

QString ButtonsDialog::buttonText(QDialogButtonBox::StandardButton sb) const
{
    QPushButton * pButton = m_buttons->button(sb);
    return pButton ? pButton->text() : QString();
}

void ButtonsDialog::setDefaultButton(QDialogButtonBox::StandardButton sb)
{
    m_defaultButton = sb;
    foreach (QAbstractButton * aButton, m_buttons->buttons())
        if (QPushButton * pButton = dynamic_cast<QPushButton *>(aButton))
            pButton->setDefault(m_buttons->standardButton(aButton) == m_defaultButton);
}

QDialogButtonBox::StandardButton ButtonsDialog::defaultButton() const
{
    return m_defaultButton;
}

void ButtonsDialog::setMessage(const QString & message)
{
    m_messageLabel->setText(message);
}

QString ButtonsDialog::message() const
{
    return m_messageLabel->text();
}

void ButtonsDialog::setTitle(const QString & title)
{
    setWindowTitle(title);
}

QString ButtonsDialog::title() const
{
    return windowTitle();
}

void ButtonsDialog::setIcon(const QIcon & icon)
{
    m_icon = icon;
    setWindowIcon(icon);
}

QIcon ButtonsDialog::icon() const
{
    return m_icon;
}

void ButtonsDialog::addExtraWidget(QWidget * widget)
{
    if (widget) {
        m_layout->insertWidget(1, widget);
        m_extraWidgets.append(widget);
        connect(widget, SIGNAL(destroyed()), this, SLOT(slotExtraWidgetDeleted(QObject*)));
    }
}

QList<QWidget *> ButtonsDialog::extraWidgets() const
{
    return m_extraWidgets;
}

void ButtonsDialog::setMemorizeEnabled(bool enabled)
{
    m_memorize = enabled;
}

bool ButtonsDialog::memorizeEnabled() const
{
    return m_memorize;
}

void ButtonsDialog::slotButtonClicked(QAbstractButton * button)
{
    // save the pressed button and return from the 'exec()'
    m_pressedButton = m_buttons->standardButton(button);
    accept();
}

void ButtonsDialog::slotExtraWidgetDeleted(QObject * object)
{
    QWidget * widget = static_cast<QWidget *>(object);
    m_extraWidgets.removeAll(widget);
}
