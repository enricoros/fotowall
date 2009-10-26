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

#ifndef __UrlHistoryBox_h__
#define __UrlHistoryBox_h__

#include "Shared/GroupBoxWidget.h"
#include <QList>
#include <QUrl>
class PixmapButton;

/// \brief Shows Preview of Fotowall files and emit clicks
class UrlHistoryBox : public GroupBoxWidget
{
    Q_OBJECT
    public:
        UrlHistoryBox(const QList<QUrl> & urls, QWidget * parent = 0);
        ~UrlHistoryBox();

    Q_SIGNALS:
        void urlClicked(const QUrl & url);

    private:
        QList<PixmapButton *> m_entries;
        int m_previewIndex;

    private Q_SLOTS:
        void slotClicked();
        void slotNextPreview();
};

#endif
