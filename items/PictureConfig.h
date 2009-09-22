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

#ifndef __PictureConfig_h__
#define __PictureConfig_h__

#include "AbstractConfig.h"
#include "PictureEffect.h"
class QListWidgetItem;
class PictureContent;
namespace Ui { class PictureConfig; }


class PictureConfig : public AbstractConfig {
    Q_OBJECT
    public:
        PictureConfig(PictureContent * pictureContent, QGraphicsItem * parent = 0);
        ~PictureConfig();

    Q_SIGNALS:
        void applyEffect(const PictureEffect & effect, bool allPictures);

    private:
        Ui::PictureConfig * m_pictureUi;
        PictureContent *    m_pictureContent;
        PictureEffect       m_currentEffect;

    private Q_SLOTS:
        void on_applyEffects_clicked();
        void on_effectsList_itemActivated(QListWidgetItem * item);
};

#endif
