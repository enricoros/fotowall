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

#include "UrlHistoryBox.h"

#include "Canvas/Canvas.h"
#include "Shared/GlowEffectWidget.h"
#include "Shared/PixmapButton.h"
#include "FotowallFile.h"

#include <QHBoxLayout>
#include <QFile>
#include <QMenu>
#include <QToolButton>
#include <QTimer>
#include <QVariant>

UrlHistoryBox::UrlHistoryBox(const QList<QUrl> &urls, QWidget *parent)
  : GroupBoxWidget(parent)
  , m_previewIndex(0)
{
    // skip if empty
    if (urls.isEmpty())
        return;

    // add buttons
    QHBoxLayout * lay = new QHBoxLayout(this);
    lay->setContentsMargins(2, 0, 0, 0);
    lay->setSpacing(0);
    for (int i = 0; i < qMin(5, urls.size()); i++) {
        const QUrl & url = urls[i];
        PixmapButton * button = new PixmapButton(QSize(64, 60), this);
        connect(button, SIGNAL(clicked()), this, SLOT(slotClicked()));
        connect(button, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotContextMenu(const QPoint &)));
        button->setContextMenuPolicy(Qt::CustomContextMenu);
        button->setProperty("url", url);
        button->setHoverText(QString::number(i+1));
        button->setToolTip(url.toString());
        lay->addWidget(button);
        m_entries.append(button);
    }

    // add load button
    PixmapButton * loadButton = new PixmapButton(QSize(64, 60), this);
    connect(loadButton, SIGNAL(clicked()), this, SIGNAL(openFile()));
    loadButton->setToolTip(tr("Load"));
    loadButton->setPixmap(QPixmap(":/data/action-open.png"));
    lay->addWidget(loadButton);

    // start preview jobs
    QTimer::singleShot(100, this, SLOT(slotNextPreview()));
}

UrlHistoryBox::~UrlHistoryBox()
{
    qDeleteAll(m_entries);
    m_entries.clear();
}

QUrl UrlHistoryBox::urlForEntry(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return QUrl();
    return m_entries[index]->property("url").toUrl();
}

void UrlHistoryBox::slotClicked()
{
    emit urlClicked(sender()->property("url").toUrl());
}

void UrlHistoryBox::slotContextMenu(const QPoint & widgetPos)
{
    // get data from the sender button
    PixmapButton * button = static_cast<PixmapButton *>(sender());
    QPoint screenPos = button->mapToGlobal(widgetPos);
    QUrl fileUrl = button->property("url").toUrl();
    QString fileString = fileUrl.toLocalFile();
    if (!QFile::exists(fileString))
        return;

    // build menu
    QMenu menu;
    QAction * openAction = menu.addAction(tr("Open"));
    menu.addSeparator();
    QAction * removeAction = menu.addAction(tr("Remove From History"));
    QMenu * fileSubMenu = menu.addMenu(tr("File"));
    QAction * deleteAction = fileSubMenu->addAction(QIcon(":/data/action-delete.png"), tr("Delete File"));

    // popup menu and handle actions
    QAction * action = menu.exec(screenPos);
    if (action == openAction) {
        emit urlClicked(fileUrl);
    } else if (action == removeAction) {
        m_entries.removeAll(button);
        button->deleteLater();
    } else if (action == deleteAction) {
        if (QFile::remove(fileString)) {
            m_entries.removeAll(button);
            button->deleteLater();
        } else
            qWarning("UrlHistoryBox::slotContextMenu: cannot remove file");
    }
}

void UrlHistoryBox::slotNextPreview()
{
    if (m_previewIndex >= m_entries.size())
        return;
    int currentIndex = m_previewIndex++;
    QUrl currentUrl = m_entries[currentIndex]->property("url").toUrl();
    QString fileName = currentUrl.toLocalFile();

    // get the embedded preview
    QImage previewImage = FotowallFile::embeddedPreview(fileName);

    // generate preview
    if (previewImage.isNull()) {
        Canvas * canvas = new Canvas(physicalDpiX(), physicalDpiY(), this);
        if (FotowallFile::read(fileName, canvas, false)) {
            // render canvas, rotate, drop shadow and set
            canvas->resizeAutoFit();
            previewImage = canvas->renderedImage(QSize(48, 48), Qt::KeepAspectRatio, true);
        }
        delete canvas;
    }

    // make a pretty preview (rotated and shadowed)
    if (!previewImage.isNull()) {
        QTransform rot;
         int mag = (qrand() % 7) + (qrand() % 7);
         static bool dir = true; dir = !dir;
         rot.rotate(dir ? mag : -mag);
        const QImage rotated = previewImage.transformed(rot, Qt::SmoothTransformation);
        const QImage shadowed = GlowEffectWidget::dropShadow(rotated, QColor(64, 64, 64), 6, 1, 1);
        m_entries[currentIndex]->setPixmap(QPixmap::fromImage(shadowed));
    }

    // start next job right after
    if (m_previewIndex < m_entries.size())
        QTimer::singleShot(10, this, SLOT(slotNextPreview()));
}
