/***************************************************************************
 *                                                                         *
 *   This file is part of the FotoWall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2007-2009 by Tanguy Arnaud <arn.tanguy@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __Export__
#define __Export__

#include "ui_ExportDialog.h"

class Desk;

class Export : public QDialog
{
    Q_OBJECT
    public:
        Export(Desk *desk);

    private:
        Ui::ExportDialog m_ui;
        Desk *m_desk;
        QSizeF m_printSize; // the print size in inches
        void setBackground();

    public Q_SLOTS:
        void slotSaveImage();
        void slotChoosePath();

        void slotPrint();
        void slotPrintUnityChanged(int);
        void slotPrintWidthChanged(double);
        void slotPrintHeightChanged(double);
};


#endif
