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

#include "CanvasModeInfo.h"

CanvasModeInfo::CanvasModeInfo()
  : m_printDpi(300)
  , m_landscape(false)
  , m_projectMode(ModeNormal)
{
}

void CanvasModeInfo::setFixedSizeInches(const QSizeF & size)
{
    // 1 inch = 2.54 cm
    //w /= (float)2.54; h /= (float)2.54;
    //m_realSizeInches = QSizeF(w, h);
    m_realSizeInches = size;
}

bool CanvasModeInfo::fixedSize() const
{
    return !m_realSizeInches.isEmpty();
}

QSizeF CanvasModeInfo::fixedSizeInches() const
{
    return m_realSizeInches;
}

QSize CanvasModeInfo::fixedScreenPixels() const
{
    return QSize(m_realSizeInches.width() * m_canvasDpi.x(), m_realSizeInches.height() * m_canvasDpi.y());
}

QSize CanvasModeInfo::fixedPrinterPixels() const
{
    return QSize(m_realSizeInches.width() * m_printDpi, m_realSizeInches.height() * m_printDpi);
}

void CanvasModeInfo::setScreenDpi(float dpiX, float dpiY)
{
    m_canvasDpi = QPointF(dpiX, dpiY);
}

QPointF CanvasModeInfo::screenDpi() const
{
    return m_canvasDpi;
}

void CanvasModeInfo::setPrintDpi(float dpi)
{
    m_printDpi = dpi;
}

float CanvasModeInfo::printDpi() const
{
    return m_printDpi;
}

void CanvasModeInfo::setPrintLandscape(bool landscape)
{
    m_landscape = landscape;
}

bool CanvasModeInfo::printLandscape() const
{
    return m_landscape;
}

void CanvasModeInfo::setProjectMode(Mode mode)
{
    m_projectMode = mode;
}

CanvasModeInfo::Mode CanvasModeInfo::projectMode() const
{
    return m_projectMode;
}
