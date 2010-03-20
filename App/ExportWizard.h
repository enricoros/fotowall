/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
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
class Canvas;
namespace Ui { class ExportWizard; }

class ExportWizard : public QWizard {
    Q_OBJECT
    public:
        ExportWizard(Canvas * canvas, bool printPreferred);
        ~ExportWizard();

        // the main functions
        void setWallpaper();
        void saveImage();
        void startPosterazor();
        bool pdf();
        void print();
        void saveSvg();

    protected:
        // manually sets a page
        void setPage(int pageId);

        // ::QWizard
        int nextId() const;

    private:
        enum PageCode { PageMode = 0, PageWallpaper = 1, PageImage = 2,
            PagePosteRazor = 3, PagePrint = 4, PageSvg = 5 };
        bool printCanvasAsImage(int printerDpi, const QSize & pixelSize, bool landscape,
                                Qt::AspectRatioMode aspectRatioMode) const;
        Ui::ExportWizard * m_ui;
        Canvas * m_canvas;
        bool m_printPreferred;
        bool m_printPdf;
        int m_nextId;
        QSizeF m_printSizeInches;

    private slots:
        // contents related
        void slotChoosePath();
        void slotChoosePdfPath();
        void slotChooseSvgPath();
        void slotImageFromCanvas();
        void slotImageFromDpi();
        void slotPrintUnityChanged(int);
        void slotPrintSizeChanged();

        // wizard related
        void slotFinished(int);
        void slotModeButtonClicked();
        void slotOpenLink(const QString & address);
};

#endif
