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

#ifndef __ModeInfo__
#define __ModeInfo__

#include <QSizeF>
#include <QPointF>

/*
 * \brief Hold infos about the current mode
 */
class ModeInfo
{
    public:
        ModeInfo();

        void setCanvasDpi(float, float);
        QPointF canvasDpi() const;
        void setPrintDpi(float);
        float printDpi() const;

        void setRealSizeCm(float, float);
        void setRealSizeInches(float, float);
        QSizeF realSize() const; //returns the real size in inches

        // Convert the real size into a pixel size according to the dpi
        QSize canvasPixelSize() const;
        QSize printPixelSize() const;

        void setLandscape(bool);
        bool landscape() const;

    private:
        QSizeF m_realSize; // Store the size in inches
        QPointF m_canvasDpi; // Store the X and Y dpi
        float m_printDpi;

        bool m_landscape;
};

#endif

