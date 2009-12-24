/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   Copyright (C) 2009 by Enrico Ros                                      *
 *         2007-2009 Enrico Ros <enrico.ros@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __FloodGenerator_h__
#define __FloodGenerator_h__

#include "FloodPoly.h"

class FloodGenerator {
    public:
        // loaders
        static FloodPoly fromQPainterPath( const QPainterPath & painterPath );
        static FloodPolys fromSvgPaths( const QString & fileName, const QString & elementId = QString() );

        // generators
        static FloodPolys starPolys( const FloodPolys & source, double mag = 0.1 );
        static FloodPolys spreadPolys( const FloodPolys & source, const QRect & outGemetry );
        static FloodPolys heavyPolys( const FloodPolys & source, double G, double dT );
};

#endif
