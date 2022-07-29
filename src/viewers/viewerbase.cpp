#include "viewers/viewerbase.h"
#include "misc/geo_graphics.h"
#include "geometry/edgepoly.h"
#include "figures/figure.h"
#include "tile/feature.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

void  ViewerBase::drawFeature(GeoGraphics * gg, FeaturePtr feature, QBrush brush, QPen pen)
{
    EdgePoly ep   = feature->getEdgePoly();

    // Fill the feature.
    if (brush.style() != Qt::NoBrush)
    {
        gg->fillEdgePoly(ep, brush.color());
    }

    // Outline the feature.
    gg->drawEdgePoly(ep,pen.color(), pen.width());
}

void  ViewerBase ::drawFigure(GeoGraphics * gg, FigurePtr figure, QPen pen)
{
    MapPtr map = figure->getFigureMap();
    if (!map) return;

    for(auto edge :  map->getEdges())
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
