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
        PixmapButton * button = new PixmapButton(QSize(80, 60));
        connect(button, SIGNAL(clicked()), this, SLOT(slotClicked()));
        button->setProperty("url", url);
        button->setHoverText(QString::number(i+1));
        button->setToolTip(url.toString());
        lay->addWidget(button);
        m_entries.append(button);
    }

    // start preview jobs
    QTimer::singleShot(500, this, SLOT(slotNextPreview()));
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

void UrlHistoryBox::slotNextPreview()
{
    if (m_previewIndex >= m_entries.size())
        return;
    int currentIndex = m_previewIndex++;
    QUrl currentUrl = m_entries[currentIndex]->property("url").toUrl();

    // generate preview (### preview size???)
    Canvas * canvas = new Canvas(this);
    if (FotowallFile::read(currentUrl.toString(), canvas, false)) {
        // render canvas, rotate, drop shadow and set
        canvas->resizeAutoFit();
        const QImage image = canvas->renderedImage(QSize(60, 45), Qt::KeepAspectRatio, true);
        QTransform rot;
         int mag = (qrand() % 7) + (qrand() % 7);
         static bool dir = true; dir = !dir;
         rot.rotate(dir ? mag : -mag);
        const QImage rotated = image.transformed(rot, Qt::SmoothTransformation);
        const QImage preview = GlowEffectWidget::dropShadow(rotated, Qt::darkGray, 6, 1, 1);
        m_entries[currentIndex]->setPixmap(QPixmap::fromImage(preview));

        // save preview to cache ;-)
        // TODO ###
    }
    delete canvas;

    // start next job right after
    if (m_previewIndex < m_entries.size())
        QTimer::singleShot(50, this, SLOT(slotNextPreview()));
}
