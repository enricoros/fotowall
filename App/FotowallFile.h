/*****************************************************************************
* Copyright (C) 2008 TANGUY Arnaud <arn.tanguy@gmail.com>                    *
*                                                                            *
* This program is free software; you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation; either version 2 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License along    *
* with this program; if not, write to the Free Software Foundation, Inc.,    *
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.                *
******************************************************************************/

#ifndef __FotowallFile_h__
#define __FotowallFile_h__

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
class CanvasModeInfo;
class Canvas;

class FotowallFile
{
    public:
        // Loading
        static bool read(const QString & filePath, Canvas * canvas);
        static void readContent(Canvas * canvas, QDomElement & parentElement);

        // Saving
        static bool save(const QString & filePath, const Canvas * canvas);
        void saveContent(const Canvas *);
        void saveCanvas(const Canvas *);
        void saveProject(const CanvasModeInfo *);
        bool writeFile(const QString & filePath);

    private:
        QDomDocument doc;               // write
        QDomElement m_rootElement;      // write
        QDomElement m_projectElement;   // shared
        QDomElement m_canvasElement;    // shared
        QDomElement m_contentElement;   // shared
};

#endif
