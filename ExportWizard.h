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

#ifndef __ExportWizard_h__
#define __ExportWizard_h__

#include <QtGui/QWizard>
class Desk;
namespace Ui { class ExportWizard; }

class ExportWizard : public QWizard {
    Q_OBJECT
    public:
        ExportWizard(Desk * desk);
        ~ExportWizard();

        // the main functions
        void setWallpaper();
        void saveImage();
        void startPosterazor();
        void print();

        // manually sets a page
        void setPage(int pageId);

        // ::QWizard
        int nextId() const;

    private:
        enum PageCode { PageMode = 0, PageWallpaper = 1, PageImage = 2, PagePosteRazor = 3, PagePrint = 4 };
        Ui::ExportWizard * m_ui;
        Desk * m_desk;
        int m_nextId;
        QSizeF m_printSize; // the print size in inches

    private slots:
        // contents related
        void slotChoosePath();
        void slotPrintUnityChanged(int);
        void slotPrintWidthChanged(double);
        void slotPrintHeightChanged(double);

        // wizard related
        void slotFinished(int);
        void slotModeButtonClicked();
        void slotOpenLink(const QString & address);
};

#endif
