/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://code.google.com/p/fotowall                                 *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros <enrico.ros@gmail.com>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __PictureEffect_h__
#define __PictureEffect_h__

#include <QtGlobal>

struct PictureEffect {
    enum Effect {
        ClearEffects    = -1,
        FlipH           =  1,
        FlipV           =  2,
        InvertColors    =  3,
        NVG             =  4,
        BlackAndWhite   =  5,
        Glow            =  6,
        Sepia           =  7,
        Opacity         =  8,
        Crop            =  9
    } effect;
    qreal param;
    QRect cropingRect;

    PictureEffect(Effect effect = ClearEffects, qreal param = 0.0, QRect cropingRect = QRect())
      : effect(effect)
      , param(param)
      , cropingRect(cropingRect)
    {
    }
};

#endif
