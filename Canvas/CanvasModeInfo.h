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

#ifndef __CavasModeInfo_h__
#define __CavasModeInfo_h__

#include <QDomElement>
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QSizeF>
#include <QString>

class CanvasModeInfo
{
public:
  enum Mode
  {
    ModeNormal = 0,
    ModeCD = 1,
    ModeDVD = 2,
    ModeExactSize = 3,
    ModeWallpaper = 4
    // NOTE: always add new consts to the end, otherwise breaks (prev) XMLs
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
  Mode previousProjectMode() const;

  // storage
  void toXml(QDomElement & canvasModeElement) const;
  void fromXml(QDomElement & canvasModeElement);

  QString projectModeName() const { return modeName(m_projectMode); }

  QString previousProjectModeName() const { return modeName(m_previousProjectMode); }

  static QString modeName(const Mode mode)
  {
    switch(mode)
    {
      case ModeNormal:
        return QObject::tr("Normal");
      case ModeCD:
        return QObject::tr("CD");
      case ModeDVD:
        return QObject::tr("DVD");
      case ModeExactSize:
        return QObject::tr("Exact");
      case ModeWallpaper:
        return QObject::tr("Wallpaper");
      default:
        return QObject::tr("Unknown");
    }
  }

private:
  QSizeF m_realSizeInches;
  QPointF m_canvasDpi;
  float m_printDpi;
  bool m_landscape;
  Mode m_projectMode, m_previousProjectMode;
};

#endif
