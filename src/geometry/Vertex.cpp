////////////////////////////////////////////////////////////////////////////
//
// Vertex.java
//
// The vertex abstraction for planar maps.  A Vertex has the usual graph
// component, a list of adjacent edges.  It also has the planar component,
// a position.  Finally, there's a user data field for applications.

#include <QTransform>
#include <QtMath>
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/geo.h"

int Vertex::refs = 0;

Vertex::Vertex(const QPointF &pos )
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
void Vertex::transform(QTransform T)
{
    pt = T.map(pt);
}

qreal Vertex::getAngle(const EdgePtr & edge)
{
    VertexPtr other = edge->getOtherV(pt);
    QPointF pd      = other->pt - pt;
    Geo::normalizeD(pd);
    qreal angle     = qAtan2(pd.x(), pd.y());
    return angle;
}



