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

#ifndef __FloodPoly__
#define __FloodPoly__

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QBrush>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>
#include <QRectF>
#include "3rdparty/enricomath.h"

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

        // looks
        QPen pen() const;
        void setPen(const QPen & pen);
        QBrush brush() const;
        void setBrush(const QBrush & brush);

        // other stuff
        FloodPoly fadedTo( const FloodPoly & other, double phase ) const;

        // generate a QPainterPath from this (temp.. for rendering)
        QPainterPath toPainterPath() const;

        // debug
        void __dump();

    private:
        Nodes m_nodes;
        QPen m_pen;
        QBrush m_brush;
};

/// \brief FloodPoly collection
typedef QList< FloodPoly > FloodPolys;

#endif
