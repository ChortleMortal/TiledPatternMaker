#include <QPainter>
#include "sys/geometry/map_base.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/neighbour_map.h"

void MapBase::paint(QPainter * painter, QTransform & tr,bool shoWDirn, bool showArcCenters, bool showVertices, bool showEdges)
{
    // set pen before calling this
    //qDebug() << "MapBase::paint" <<  info() << Transform::info(tr);

    //verify("MapBase", true, true, true);

    if (showEdges)
    {
        for (auto & edge : std::as_const(edges))
        {
            QPointF p1 = tr.map(edge->v1->pt);
            QPointF p2 = tr.map(edge->v2->pt);

            //qDebug() << "edge" << p1 << p2;

            if (shoWDirn)
            {
                GeoGraphics::drawLineArrowDirect(QLineF(p1,p2),painter->pen(),painter);
            }

            if (edge->getType() == EDGETYPE_LINE)
            {
                painter->drawLine(p1,p2);
            }
            else if (edge->getType() == EDGETYPE_CURVE)
            {
                QPointF arcCenter = tr.map(edge->getArcCenter());
                ArcData ad(QLineF(p1,p2),arcCenter,edge->getCurveType());
                painter->drawArc(ad.rect(), qRound(ad.start() * 16.0),qRound(ad.span() * 16.0));
            }

            if (edge->isCurve() && showArcCenters)
            {
                QPointF pt = edge->getArcCenter();
                if (edge->getCurveType() == CURVE_CONCAVE)
                {
                    pt = Geo::reflectPoint(pt,edge->getLine());
                }
                qreal radius = 8.0;
                painter->save();
                painter->setPen(QPen(Qt::blue,3.0));
                painter->setBrush(Qt::NoBrush);
                painter->drawEllipse(tr.map(pt), radius, radius);
                painter->restore();
            }
        }
    }

    if (showVertices)
    {
        qreal radius = 4.0;
        painter->setPen(QPen(Qt::blue,1));
        painter->setBrush(Qt::blue);
        for (const VertexPtr & v : std::as_const(vertices))
        {
            QPointF pos = tr.map(v->pt);
            painter->drawEllipse(pos, radius, radius);
        }
    }
}

// Applying a motion made up only of uniform scales and translations,
// Angles don't change.  So we can just transform each vertex.
void MapBase::transform(const QTransform & T)
{
    for (const auto & vert : std::as_const(vertices))
    {
        vert->setPt(T.map(vert->pt));
    }
    for (const auto & edge : std::as_const(edges))
    {
        if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF pt = T.map(edge->getArcCenter());
            edge->chgangeToCurvedEdge(pt,edge->getCurveType());
        }
    }
}

void MapBase::wipeout()
{
    // better to remove edges before removing vertices
    edges.clear();          // unnecessary from destructor but not elsewhere
    vertices.clear();       // unneccesary from destructor but not elsewhere
}

bool MapBase::isEmpty() const
{
    return  (vertices.size() < 2);
}

QString MapBase::info() const
{
    return QString("vertices=%1 edges=%2").arg(vertices.size()).arg(edges.size());
}
