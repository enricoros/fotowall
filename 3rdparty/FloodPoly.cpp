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

#include "FloodPoly.h"
#include <QDebug>

FloodPoly::FloodPoly()
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
