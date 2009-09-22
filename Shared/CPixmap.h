/****************************
* Auteur :Tanguy Arnaud
* Date : 2008
*
* Copyright (C) 2008 TANGUY Arnaud
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ******************************/

#ifndef ARNAUD_H_CPIXMAP
#define ARNAUD_H_CPIXMAP

#include <QPixmap>
#include "PictureEffect.h"

class CPixmap : public QPixmap {
public:
   CPixmap();
   CPixmap(const QString &fileName);
   CPixmap(const QImage &image);

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
   void toSepia();  // Old photo style
   void toCropped(const QRect &cropRect);
   //void toLuminosity(int value);

private:
    CPixmap(const QPixmap &pixmap);
    void updateImage(QImage &newImage);

    QImage m_image;
    QString m_filePath;
    // Ordered list of currently applied effects
    QList<PictureEffect> m_effects;
};

#endif /* ARNAUD_H_CPIXMAP */
