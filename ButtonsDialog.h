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

#ifndef __ButtonsDialog_h__
#define __ButtonsDialog_h__

#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QList>
#include <QMap>
class QLabel;
class QVBoxLayout;


class ButtonsDialog : public QDialog
{
    Q_OBJECT
    public:
        ButtonsDialog(const QString & dialogId, QWidget * parent = 0);

        QDialogButtonBox::StandardButton execute();

        void setButtons(QDialogButtonBox::StandardButtons buttons);
        QDialogButtonBox::StandardButtons buttons() const;

        void setButtonText(QDialogButtonBox::StandardButton button, const QString & text);
        QString buttonText(QDialogButtonBox::StandardButton button) const;

        void setDefaultButton(QDialogButtonBox::StandardButton button);
        QDialogButtonBox::StandardButton defaultButton() const;

        void setMessage(const QString & message);
        QString message() const;

        void setTitle(const QString & title);
        QString title() const;

        void setIcon(const QIcon & icon);
        QIcon icon() const;

        void addExtraWidget(QWidget * widget);
        QList<QWidget *> extraWidgets() const;

        void setMemorizeEnabled(bool enabled);
        bool memorizeEnabled() const;

    private:
        QString m_dialogId;
        bool m_memorize;
        QVBoxLayout * m_layout;
        QLabel * m_messageLabel;
        QDialogButtonBox * m_buttons;
        QIcon m_icon;
        QList<QWidget *> m_extraWidgets;
        QDialogButtonBox::StandardButton m_defaultButton;
        QDialogButtonBox::StandardButton m_pressedButton;
        QMap<QDialogButtonBox::StandardButton, QString> m_buttonNames;

    private Q_SLOTS:
        void slotButtonClicked(QAbstractButton *);
        void slotExtraWidgetDeleted(QObject * object);
};

#endif
