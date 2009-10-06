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

#ifndef __CavasModeInfo_h__
#define __CavasModeInfo_h__

#include <QSizeF>
#include <QSize>
#include <QPointF>
#include <QDomElement>

class CanvasModeInfo
{
    public:
        enum Mode {
            ModeNormal      = 0,
            ModeCD          = 1,
            ModeDVD         = 2,
            ModeExactSize   = 3
        };
        CanvasModeInfo();

        void setFixedSizeInches(const QSizeF & size = QSizeF());
        bool fixedSize() const;
        QSizeF fixedSizeInches() const;
        QSize fixedScreenPixels() const;
        QSize fixedPrinterPixels() const;

        void setScreenDpi(float dpiX, float dpiY);
        QPointF screenDpi() const;

        void setPrintDpi(float dpi);
        float printDpi() const;

        void setPrintLandscape(bool landscape);
        bool printLandscape() const;

        void setProjectMode(Mode mode);
        Mode projectMode() const;

        // storage
        void toXml(QDomElement & parentElement) const;
        void fromXml(QDomElement & parentElement);

    private:
        QSizeF m_realSizeInches;
        QPointF m_canvasDpi;
        float m_printDpi;
        bool m_landscape;
        Mode m_projectMode;
};

#endif

