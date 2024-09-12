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
#include <QStyle>
class QCheckBox;
class QVBoxLayout;
class QLabel;

class ButtonsDialog : public QDialog
{
  Q_OBJECT
public:
  ButtonsDialog(const QString & dialogId,
                const QString & title = QString(),
                const QString & message = QString(),
                QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::NoButton,
                bool buttonsCentered = false,
                bool memorize = false);

  QDialogButtonBox::StandardButton execute();

  void setButtons(QDialogButtonBox::StandardButtons buttons);
  QDialogButtonBox::StandardButtons buttons() const;

  void setButtonText(QDialogButtonBox::StandardButton button, const QString & text);
  QString buttonText(QDialogButtonBox::StandardButton button) const;

  void setDefaultButton(QDialogButtonBox::StandardButton button);
  QDialogButtonBox::StandardButton defaultButton() const;

  void setButtonsCentered(bool centered);
  bool buttonsCentered() const;

  void setMessage(const QString & message);
  QString message() const;

  void setTitle(const QString & title);
  QString title() const;

  void setIcon(const QIcon & icon);
  void setIcon(QStyle::StandardPixmap icon);
  QIcon icon() const;

  void addExtraWidget(QWidget * widget);
  QList<QWidget *> extraWidgets() const;

  void setMemorizeEnabled(bool enabled);
  bool memorizeEnabled() const;

private:
  QString m_dialogId;
  QIcon m_icon;
  bool m_memorize;
  QLabel * m_iconLabel;
  QLabel * m_messageLabel;
  QCheckBox * m_memorizeCheckbox;
  QVBoxLayout * m_extraLayout;
  QList<QWidget *> m_extraWidgets;
  QDialogButtonBox * m_buttonBox;
  QDialogButtonBox::StandardButton m_defaultButton;
  QDialogButtonBox::StandardButton m_pressedButton;
  QMap<QDialogButtonBox::StandardButton, QString> m_buttonNames;

private Q_SLOTS:
  void slotButtonClicked(QAbstractButton *);
  void slotExtraWidgetDeleted(QObject * object);
};

#endif
