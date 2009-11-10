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

#ifndef __FotowallFile_h__
#define __FotowallFile_h__

#include <QtGlobal>
#include <QStringList>
#include <QImage>
class Canvas;

namespace FotowallFile
{
    // read a .fotowall file and setup the Canvas
    bool read(const QString & filePath, Canvas * canvas, bool inHistory);
    QImage embeddedPreview(const QString & filePath);

    // save the given Canvas to a .fotowall file
    bool saveV2(const QString & filePath, const Canvas * canvas);

    // file selector dialogs
    QString getLoadFotowallFile();
    QStringList getLoadFotowallFiles();
    QString getSaveFotowallFile();
};

#endif
