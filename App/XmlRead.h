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

#ifndef __XmlRead__
#define __XmlRead__

#include <QObject>
#include <QDomElement>
class CanvasModeInfo;
class Canvas;

class XmlRead
{
    public:
        static bool read(const QString & filePath, Canvas * canvas, CanvasModeInfo * modeInfo);

        bool loadFile(const QString & filePath);
        void readProject(CanvasModeInfo * modeInfo);
        void readCanvas(Canvas * canvas);
        void readContent(Canvas * canvas);

    private:
        QDomElement m_projectElement;
        QDomElement m_canvasElement;
        QDomElement m_contentElement;
};

#endif
