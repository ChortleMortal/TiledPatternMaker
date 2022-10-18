#include "viewers/viewerbase.h"
#include "misc/geo_graphics.h"
#include "geometry/edgepoly.h"
#include "motifs/motif.h"
#include "tile/tile.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

void  ViewerBase::drawTile(GeoGraphics * gg, TilePtr tile, QBrush brush, QPen pen)
{
    EdgePoly ep   = tile->getEdgePoly();

    // Fill the tile.
    if (brush.style() != Qt::NoBrush)
    {
        gg->fillEdgePoly(ep, brush.color());
    }

    // Outline the tile.
    gg->drawEdgePoly(ep,pen.color(), pen.width());
}

void  ViewerBase ::drawMotif(GeoGraphics * gg, MotifPtr motif, QPen pen)
{
    MapPtr map = motif->getMap();
    if (!map) return;

    for(auto & edge :  qAsConst(map->getEdges()))
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            gg->drawArc(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),pen);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            gg->drawChord(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),pen);
        }
    }
}
