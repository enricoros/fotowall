/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "VersionCheckDialog.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTimer>
#include <QUrl>
#include "ui_VersionCheckDialog.h"

#define MAGIC_TOKEN "Last version is "
#define DOWNLOADS_URL "http://code.google.com/p/fotowall/downloads/list"

VersionCheckDialog::VersionCheckDialog(QWidget * parent)
  : QDialog(parent)
  , ui(new Ui::VersionCheckDialog)
  , m_nam(new QNetworkAccessManager)
{
    // inject ui
    ui->setupUi(this);
    ui->currVersion->setText(QCoreApplication::applicationVersion());
    ui->nextVersion->setText(tr("checking"));

    // start the network request
    QNetworkRequest request(QUrl("http://code.google.com/p/fotowall/"));
    QNetworkReply * reply = m_nam->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(slotGotReply()));
    QTimer::singleShot(5000, this, SLOT(slotTimeOut()));
}

VersionCheckDialog::~VersionCheckDialog()
{
    delete m_nam;
    delete ui;
}

void VersionCheckDialog::slotGotReply()
{
    if (!m_nam)
        return;
    QNetworkReply * reply = static_cast<QNetworkReply *>(sender());
    QString replyStr = reply->readAll();
    ui->progressBar->hide();
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        ui->nextVersion->setText("network error");
        reply->deleteLater();
        return;
    }

    QString magic(MAGIC_TOKEN);
    int pos1 = replyStr.indexOf(magic) + magic.length();
    if (pos1 == -1 || pos1 >= (replyStr.length() - 1)) {
        ui->nextVersion->setText("page error (1)");
        return;
    }

    int pos2 = replyStr.indexOf("<", pos1);
    if (pos2 <= pos1 || pos2 > pos1 + 20) {
        ui->nextVersion->setText("page error (2)");
        return;
    }

    // show version
    QString version = replyStr.mid(pos1, pos2 - pos1);
    ui->nextVersion->setText(version);

    // show download button
    if (version.compare(QCoreApplication::applicationVersion(), Qt::CaseInsensitive)) {
        QPushButton * dlButton = ui->buttonBox->addButton(tr("Download"), QDialogButtonBox::NoRole);
        connect(dlButton, SIGNAL(clicked()), this, SLOT(slotDownload()));
    }
}

void VersionCheckDialog::slotTimeOut()
{
    m_nam->deleteLater();
    m_nam = 0;
    ui->progressBar->hide();
    ui->nextVersion->setText(tr("network timeout"));
}

void VersionCheckDialog::slotDownload()
{
    QDesktopServices::openUrl(QUrl(DOWNLOADS_URL));
}
