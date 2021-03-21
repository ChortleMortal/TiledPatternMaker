/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////
//
// Vertex.java
//
// The vertex abstraction for planar maps.  A Vertex has the usual graph
// component, a list of adjacent edges.  It also has the planar component,
// a position.  Finally, there's a user data field for applications.

#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/point.h"
#include <QTransform>

int Vertex::refs = 0;

Vertex::Vertex( QPointF pos ) : Neighbours(this)
{
    refs++;
    this->pt        = pos;
    visited         = false;
}

Vertex::~Vertex()
{
    refs--;
    //qDebug() << "Vertex destructor";
    copy.reset();
}

// Apply a transform.  Recalculate all the angles.  The order
// doesn't change, although the list might need to get reversed
// if the transform flips.  CSKFIXME -- make this work.
// Fortunately, the rigid motions we'll apply in Islamic design
// won't contain flips.  So we're okay for now.
// casper: don't need to recalc angle
void Vertex::applyRigidMotion(QTransform T)
{
    pt = T.map(pt);
}

qreal Vertex::getAngle(EdgePtr edge)
{
    VertexPtr other = edge->getOtherV(pt);
    QPointF pd      = other->pt - pt;
    Point::normalizeD(pd);
    qreal angle     = qAtan2(pd.x(), pd.y());
    return angle;
}



