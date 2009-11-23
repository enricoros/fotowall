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

#ifndef __FloodPoly__
#define __FloodPoly__

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QPainterPath>
#include <QRectF>
#include "enricomath.h"

// TODO: ADD A NOTION OF LINE-FOLLOWING for Rendering Segments

/// \brief Basic editable path, the atom of this framework
class FloodPoly {
    public:
        FloodPoly();

        // definitions
        struct Node {
            Vector2     point;
            Control2    control;
        };
        typedef QList< Node > Nodes;

        // structure manipulations
        void addNode( const Node & node );
        void addNode( const Vector2 & point, const Control2 & control );
        Node takeFirst();
        Node takeLast();
        void reset();
        int nodes() const;

        // aspect manipulation
        void setPos( double px, double py );
        void moveBy( double tx, double ty );
        void scale( const Vector2 & pivot, double xMag, double yMag );
        QRectF pointsBoundingRect() const;
        Vector2 centerVector() const;

        // direct point editing
        Nodes & edit();
        const Nodes & view() const;

        // other stuff
        FloodPoly fadedTo( const FloodPoly & other, double phase ) const;

        // generate a QPainterPath from this (temp.. for rendering)
        QPainterPath toPainterPath() const;

        // debug
        void __dump();

    private:
        Nodes m_nodes;
};

/// \brief FloodPoly collection
typedef QList< FloodPoly > FloodPolys;

#endif
