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

#ifndef ARNAUD_H_CPIXMAP
#define ARNAUD_H_CPIXMAP

#include "PictureEffect.h"
#include <QPixmap>

class CPixmap : public QPixmap
{
public:
  CPixmap();
  CPixmap(const QString & filePath);
  CPixmap(const QImage & image);

  // effects
  void addEffect(const PictureEffect & effect);
  void clearEffects();

  // the ordered sequence of effects
  QList<PictureEffect> effects() const;

  // manual operations
  void toNVG();
  void toInvertedColors();
  void toHFlip();
  void toVFlip();
  void toBlackAndWhite();
  void toGlow(int radius);
  void toSepia(); // Old photo style
  void toCropped(const QRect & cropRect);
  void toAutoBlend(qreal strength);
  void rotate();
  // void toLuminosity(int value);

private:
  CPixmap(const QPixmap & pixmap);
  void updateImage(const QImage & newImage);

  QImage m_image;
  QString m_filePath;
  // Ordered list of currently applied effects
  QList<PictureEffect> m_effects;
};

#endif /* ARNAUD_H_CPIXMAP */
