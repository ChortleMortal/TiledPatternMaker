////////////////////////////////////////////////////////////////////////////
//
// Vertex.java
//
// The vertex abstraction for planar maps.  A Vertex has the usual graph
// component, a list of adjacent edges.  It also has the planar component,
// a position.  Finally, there's a user data field for applications.

#include <QTransform>
#include <QtMath>
#include "sys/geometry/vertex.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"

int Vertex::refs = 0;

Vertex::Vertex(const QPointF &pos )
{
    refs++;
    pt      = pos;
    visited = false;
}

Vertex::~Vertex()
{
    refs--;
    //qDebug() << "Vertex destructor";
    copy.reset();
}

qreal Vertex::getAngle(const EdgePtr & edge)
{
    VertexPtr other = edge->getOtherV(pt);
    QPointF pd      = other->pt - pt;
    Geo::normalizeD(pd);
    qreal angle     = qAtan2(pd.x(), pd.y());
    return angle;
}

bool  Vertex::equals(VertexPtr & other)
{
    return Loose::equalsPt(pt,other->pt);
}

