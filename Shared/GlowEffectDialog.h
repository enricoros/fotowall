/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by TANGUY Arnaud <arn.tanguy@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ARNAUD_GLOWEFFECTDIALOG_H
#define ARNAUD_GLOWEFFECTDIALOG_H

#include <QDialog>
#include "ui_GlowEffectDialog.h"
#include "GlowEffectWidget.h"

class GlowEffectDialog : public QDialog
{
    public:
        GlowEffectDialog(const QImage & previewImage);

        int currentRadius() const;
        QImage glow(const QImage & image, int radius) const;

    private:
        Ui::GlowEffectDialog ui;
};

#endif

