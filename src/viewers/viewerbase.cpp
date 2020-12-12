#include "viewers/viewerbase.h"
#include "viewers/geo_graphics.h"
#include "geometry/edgepoly.h"
#include "tapp/figure.h"
#include "tile/feature.h"

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
    for(auto edge :  map->getEdges())
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            gg->drawChord(edge->getV1()->getPosition(),edge->getV2()->getPosition(),edge->getArcCenter(),pen,QBrush(),edge->isConvex());
        }
    }
}
