#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/viewer_services.h"
#include "model/motifs/extended_boundary.h"
#include "model/motifs/extender.h"
#include "model/motifs/motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/map.h"
#include "sys/geometry/vertex.h"

void  ViewerServices::drawTile(GeoGraphics * gg, TilePtr tile, QBrush brush, QPen pen)
{
    const EdgePoly & ep   = tile->getEdgePoly();

    // Fill the tile.
    if (brush.style() != Qt::NoBrush)
    {
        QPen pen2(brush.color());
        gg->fillEdgePoly(ep, pen2);
    }

    // Outline the tile.
    gg->drawEdgePoly(ep,pen);
}

void  ViewerServices ::drawMotif(GeoGraphics * gg, MotifPtr motif, QPen pen)
{
    MapPtr map = motif->getValidMotifMap();
    if (!map) return;

    for(auto & edge :  std::as_const(map->getEdges()))
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            gg->drawArc(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->getCurveType(),pen,true);
        }
    }
}

void ViewerServices::drawExtendedBoundary(GeoGraphics * gg, MotifPtr motif, QPen pen)
{
    for (const ExtenderPtr & ep : std::as_const(motif->getExtenders()))
    {
        const ExtendedBoundary & eb = ep->getExtendedBoundary();

        QPainterPath path;
        if (!eb.isCircle())
        {
            path.addPolygon(eb.getPoly());
            path.closeSubpath();
        }
        else
        {
            qreal radius = eb.getRadius();
            path.addEllipse(QPointF(0,0),radius,radius);
        }

        QPainter * painter = gg->getPainter();
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(gg->getTransform().map(path));
    }
}
