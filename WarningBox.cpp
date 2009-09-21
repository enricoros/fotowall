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

#include "WarningBox.h"
#include <QApplication>
#include <QCheckBox>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>
#include <QStyle>


WarningBox::WarningBox(const QString & key, const QString & title, const QString & text)
#if QT_VERSION >= 0x040500
  : QDialog(0, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
#else
  : QDialog(0, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
#endif
{
    // skip this if asked to not repeat it
    QSettings s;
    if (s.value(key, false).toBool())
        return;

    // create contents
    QLabel * label = new QLabel(this);
    label->setTextInteractionFlags(Qt::NoTextInteraction);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label->setOpenExternalLinks(true);
    label->setContentsMargins(2, 0, 0, 0);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setText(text);

    QLabel * iconLabel = new QLabel(this);
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(32, 32));

    QCheckBox * checkBox = new QCheckBox(this);
    checkBox->setText(tr("show this warning again next time"));

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
    buttonBox->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons, 0, this));
    QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(close()));
    buttonBox->setFocus();

    QGridLayout * grid = new QGridLayout(this);
    grid->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    grid->addWidget(label, 0, 1, 1, 1);
    grid->addWidget(checkBox, 1, 1, 1, 1);
    grid->addWidget(buttonBox, 2, 0, 1, 2);

    // customize and dialog
    setWindowTitle(title);
    setModal(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    QSize screenSize = QApplication::desktop()->availableGeometry(QCursor::pos()).size();
    setMinimumWidth(qMin(screenSize.width() - 480, screenSize.width()/2));
    setMinimumHeight(190);
    grid->activate();
    adjustSize();
    resize(minimumSize());
    exec();

    // avoid popping up again, if chosen
    if (!checkBox->isChecked())
        s.setValue(key, true);
}
