/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      file             : likebackdialog.h                                *
 *      license          : GPL v2+                                         *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
                              likebackdialog.h
                             -------------------
    begin                : unknown
    imported to LB svn   : 3 june, 2009
    copyright            : (C) 2006 by Sebastien Laout
                           (C) 2008-2009 by Valerio Pilo, Sjors Gielen
    email                : sjors@kmess.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LikeBackDialog_h__
#define __LikeBackDialog_h__

#include <QDialog>
#include "LikeBack.h"
#include "ui_likebackdialog.h"
class QButtonGroup;
class QNetworkAccessManager;

class LikeBackDialog : public QDialog, private Ui::LikeBackDialog
{
    Q_OBJECT
    public:
        // Constructor
        LikeBackDialog( QNetworkAccessManager * nam, LikeBack::Button reason,
                        const QString &initialComment, const QString &windowPath,
                        const QString &context, LikeBack *likeBack );
        // Destructor
        ~LikeBackDialog();

    private:
        // Construct the introductory text of the dialog
        QString introductionText();

    private:
        // External network access manager
        QNetworkAccessManager * m_nam;
        // Additional referred window information
        QString       m_context;
        // The parent LikeBack instance
        LikeBack     *m_likeBack;
        // Group of comment type checkboxes
        QButtonGroup *m_typeGroup;
        // The id of the window this dialog refers to
        QString       m_windowPath;

    private slots:
        // Check if the UI should allow the user to send the comment
        void verifyOk();
        // Send the comment to the developers site (reimpl. from KDialog)
        void slotSendData();
        // Display confirmation of the sending action
        void slotRequestFinished();
};

#endif
