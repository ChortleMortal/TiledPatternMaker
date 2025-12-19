#include "model/styles/sketch.h"
#include "sys/geometry/transform.h"
#include <QPainter>
#include <QtGlobal>
#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
// Sketch.java
//
// One day, it occured to me that I might be able to get a sketchy
// hand-drawn effect by drawing an edge as a set of line segments whose
// endpoints are jittered relative to the original edge.  And it worked!
// Also, since the map is fixed, we can just reset the random seed every
// time we draw the map to get coherence.  Note that coherence might not
// be a good thing -- some animations work well precisely because the
// random lines that make up some object change from frame to frame (c.f.
// Bill Plympton).  It's just a design decision, and easy to reverse
// (or provide a UI for).
//
// I haven't tried it yet, but I doubt this looks any good as postscript.
// the resolution is too high and it would probably look like, well,
// a bunch of lines.



// Creation.

Sketch::Sketch(ProtoPtr proto) : Plain(proto)
{
    srand(279401L);    // TODO depecated
}

Sketch::Sketch(StylePtr other) : Plain(other)
{
    srand(279401L);    // TODO depecated
}

Sketch::~Sketch()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting sketch";
#endif
}

// Style overrrides.

void Sketch::draw(GeoGraphics * gg)
{
    if (!isVisible())
    {
        return;
    }

    MapPtr map = prototype->getProtoMap();
    if (!map)
    {
        return;
    }

    QPen pen(colors.getNextTPColor().color);

    qreal jitter = Transform::distFromInvertedZero(gg->getTransform(),5.0);
    qreal halfjit = jitter / 2.0;
    for (auto & edge : std::as_const(map->getEdges()))
    {
        QPointF a = edge->v1->pt - QPointF(halfjit,halfjit);
        QPointF b = edge->v2->pt - QPointF(halfjit,halfjit);

        for( int c = 0; c < 8; ++c )
        {
            volatile qreal r1 = (rand()/RAND_MAX) * jitter;
            volatile qreal r2 = (rand()/RAND_MAX) * jitter;
            volatile qreal r3 = (rand()/RAND_MAX) * jitter;
            volatile qreal r4 = (rand()/RAND_MAX) * jitter;
            VertexPtr v1 = make_shared<Vertex>(a + QPointF(r1,r2));
            VertexPtr v2 = make_shared<Vertex>(b + QPointF(r3,r4));
            EdgePtr edge2;
            if (edge->getType() == EDGETYPE_LINE)
            {
                edge2 = make_shared<Edge>(v1,v2);
            }
            else if (edge->getType() == EDGETYPE_CURVE)
            {
                edge2 = make_shared<Edge>(v1,v2, edge->getArcCenter(), edge->getCurveType());
            }
            gg->drawEdge(edge2,pen);
        }
    }
}


