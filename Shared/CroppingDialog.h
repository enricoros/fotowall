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

#ifndef __CroppingDialog__
#define __CroppingDialog__

#include <QDialog>
#include "ui_CroppingDialog.h"
#include "CroppingWidget.h"

class CroppingDialog : public QDialog
{
    Q_OBJECT
    public:
        CroppingDialog(QPixmap *pix);
        QRect getCroppingRect() const;
    private:
        Ui::CroppingDialog ui;
};

#endif

