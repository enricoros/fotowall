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

#include "FloodPoly.h"

FloodPoly::FloodPoly()
  : m_pen(Qt::black)
  , m_brush(Qt::NoBrush)
{
}

void FloodPoly::addNode( const Node & node )
{
    m_nodes.append( node );
}

void FloodPoly::addNode( const Vector2 & point, const Control2 & control )
{
    Node n;
    n.point = point;
    n.control = control;
    m_nodes.append( n );
}

FloodPoly::Node FloodPoly::takeFirst()
{
    return m_nodes.takeFirst();
}

FloodPoly::Node FloodPoly::takeLast()
{
    return m_nodes.takeLast();
}

void FloodPoly::reset()
{
    m_nodes.clear();
}

int FloodPoly::nodes() const
{
    return m_nodes.size();
}

void FloodPoly::setPos( double px, double py )
{
    if ( m_nodes.isEmpty() )
        return;
    const Node & firstNode = m_nodes.first();
    Vector2 tv( px - firstNode.point.x(), py - firstNode.point.y() );
    Nodes::iterator it = m_nodes.begin(), end = m_nodes.end();
    for ( ; it != end; ++it )
        it->point += tv;
}

void FloodPoly::moveBy( double tx, double ty )
{
    Vector2 tv( tx, ty );
    Nodes::iterator it = m_nodes.begin(), end = m_nodes.end();
    for ( ; it != end; ++it )
        it->point += tv;
}

void FloodPoly::scale( const Vector2 & pivot, double xMag, double yMag )
{
    Nodes::iterator it = m_nodes.begin(), end = m_nodes.end();
    for ( ; it != end; ++it ) {
        Vector2 tVector = it->point - pivot;
        it->point = pivot + Vector2( tVector.x() * xMag, tVector.y() * yMag );
    }
}

/*QRectF FloodPoly::pointsBoundingRect() const
{
    if ( m_nodes.isEmpty() )
        return QRectF();

    QRectF br;
    Nodes::const_iterator it = m_nodes.begin(), end = m_nodes.end();
    for ( ; it != end; ++it ) {
        const Node & node = *it;
        double nx = node.point.x();
        double ny = node.point.y();
        if ( br.isNull() )
            br = QRectF( nx, ny, 0, 0 );
        else {
            if ( nx < br.left() )
                br.setLeft( nx );
            else if ( nx > br.right() )
                br.setRight( nx );
            if ( ny < br.top() )
                br.setTop( ny );
            else if ( ny > br.bottom() )
                br.setBottom( ny );
        }
    }
    return br;
}*/

Vector2 FloodPoly::centerVector() const
{
    if ( m_nodes.isEmpty() )
        return Vector2(0, 0);
    Vector2 acc;
    int items = 0;
    Nodes::const_iterator it = m_nodes.begin(), end = m_nodes.end();
    for ( ; it != end; ++it ) {
        acc += it->point;
        items++;
    }
    return acc * (1.0 / (double)items);
}

FloodPoly::Nodes & FloodPoly::edit()
{
    return m_nodes;
}

const FloodPoly::Nodes & FloodPoly::view() const
{
    return m_nodes;
}

static FloodPoly::Node fadedNode( const FloodPoly::Node & a, const FloodPoly::Node & b, double phase )
{
    FloodPoly::Node fadedNode;
    fadedNode.point = a.point * (1.0 - phase) + b.point * phase;
    double cRo = a.control.ro() * (1.0 - phase) + b.control.ro() * phase;
    double cTheta = a.control.theta() * (1.0 - phase) + b.control.theta() * phase;
    fadedNode.control = Control2( cRo, cTheta );
    return fadedNode;
}

QPen FloodPoly::pen() const
{
    return m_pen;
}
void FloodPoly::setPen(const QPen & pen)
{
    m_pen = pen;
}

QBrush FloodPoly::brush() const
{
    return m_brush;
}

void FloodPoly::setBrush(const QBrush & brush)
{
    m_brush = brush;
}

FloodPoly FloodPoly::fadedTo( const FloodPoly & other, double phase ) const
{
    FloodPoly fadedLine;
    Nodes::const_iterator myIt = m_nodes.begin(), myEnd = m_nodes.end();
    const Nodes & otherNodes = other.view();
    Nodes::const_iterator hisIt = otherNodes.begin(), hisEnd = otherNodes.end();
    while ( myIt != myEnd && hisIt != hisEnd ) {
        fadedLine.addNode( fadedNode( *myIt, *hisIt, phase ) );
        ++myIt;
        ++hisIt;
    }
    return fadedLine;
}

QPainterPath FloodPoly::toPainterPath() const
{
    int nodes = m_nodes.size();
    if ( nodes < 2 )
        return QPainterPath();

    Nodes::const_iterator it = m_nodes.begin(), end = m_nodes.end();

    // create path and move to the first point
    QPainterPath path;
    Vector2 lastP = it->point;
    Control2 lastCp = it->control;
    path.moveTo( lastP.x(), lastP.y() );

    // add all other points
    for ( ++it; it != end; ++it ) {
        Vector2 cp1 = lastCp.toVector2() + lastP;
        lastP = it->point;
        lastCp = it->control;
        Vector2 cp2 = lastP - lastCp.toVector2();
        path.cubicTo( cp1.x(), cp1.y(), cp2.x(), cp2.y(), lastP.x(), lastP.y() );
    }

    // the path is complete now
    return path;
}

void FloodPoly::__dump()
{
    qWarning( "FloodPoly: %d nodes", m_nodes.count() );
    foreach ( FloodPoly::Node n, m_nodes ) {
        n.point.dump();
        n.control.dump();
    }
}
