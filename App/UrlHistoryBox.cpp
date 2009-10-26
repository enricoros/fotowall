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
    lay->setMargin(0);
    lay->setSpacing(0);
    for (int i = 0; i < qMin(5, urls.size()); i++) {
        const QUrl & url = urls[i];
        PixmapButton * button = new PixmapButton(QSize(80, 60));
        connect(button, SIGNAL(clicked()), this, SLOT(slotClicked()));
        button->setProperty("url", url);
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

    // generate preview (HARDCODED ###)
    Canvas * canvas = new Canvas(QSize(800, 600));
    if (FotowallFile::read(currentUrl.toString(), canvas, false)) {
        // render canvas
        QImage image = canvas->renderedImage(QSize(64, 48), Qt::KeepAspectRatio, true);
        // rotate a little
        image = image.transformed(QTransform().rotate(-10 + (qrand() % 21)), Qt::SmoothTransformation);
        // set as preview
        m_entries[currentIndex]->setPixmap(QPixmap::fromImage(image));
    }
    delete canvas;

    // start next job right after
    if (m_previewIndex < m_entries.size())
        QTimer::singleShot(50, this, SLOT(slotNextPreview()));
}
