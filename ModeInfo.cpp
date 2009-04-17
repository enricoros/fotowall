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

#include "ModeInfo.h"

ModeInfo::ModeInfo() : m_printDpi(300), m_landscape(false)
{
}

void ModeInfo::setDeskDpi(float dpiX, float dpiY)
{
    m_deskDpi = QPointF(dpiX, dpiY);
}
QPointF ModeInfo::deskDpi() const
{
    return m_deskDpi;
}

void ModeInfo::setPrintDpi(float dpi)
{
    m_printDpi = dpi;
}
float ModeInfo::printDpi() const
{
    return m_printDpi;
}

void ModeInfo::setRealSizeCm(float w, float h)
{
    // 1 inch = 2.54 cm
    w /= 2.54; h /= 2.54;
    m_realSize = QSizeF(w, h);
}
void ModeInfo::setRealSizeInches(float w, float h)
{
    m_realSize = QSizeF(w, h);
}
QSizeF ModeInfo::realSize() const
{
    return m_realSize;
}

QSize ModeInfo::deskPixelSize() const
{
    return QSize(m_realSize.width() * m_deskDpi.x(), m_realSize.height() * m_deskDpi.y());
}
QSize ModeInfo::printPixelSize() const
{
    return QSize(m_realSize.width() * m_printDpi, m_realSize.height() * m_printDpi);
}

void ModeInfo::setLandscape(bool landscape)
{
    m_landscape = landscape;
}
bool ModeInfo::landscape()
{
    return m_landscape;
}

