/***************************************************************************
 * This file is part of the Vecta project                                  *
 *                                                                         *
 * Copyright (c) 2009 Enrico Ros                                           *
 *         2007-2009 Enrico Ros <enrico.ros@gmail.com>                     *
 *                                                                         *
 * Permission is hereby granted, free of charge, to any person             *
 * obtaining a copy of this software and associated documentation          *
 * files (the "Software"), to deal in the Software without                 *
 * restriction, including without limitation the rights to use,            *
 * copy, modify, merge, publish, distribute, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the               *
 * Software is furnished to do so, subject to the following                *
 * conditions:                                                             *
 *                                                                         *
 * The above copyright notice and this permission notice shall be          *
 * included in all copies or substantial portions of the Software.         *
 *                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,         *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES         *
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT             *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,            *
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING            *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR           *
 * OTHER DEALINGS IN THE SOFTWARE.                                         *
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
