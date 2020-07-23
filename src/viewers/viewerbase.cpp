#include "viewers/viewerbase.h"
#include "viewers/geo_graphics.h"
#include "base/utilities.h"
#include "geometry/edgepoly.h"
#include "tapp/figure.h"
#include "tile/feature.h"

void  ViewerBase::drawFeature(GeoGraphics * gg, FeaturePtr fp, QBrush brush, QPen pen)
{
    qDebug() << "draw feature" << Utils::addr(fp.get());

    // Fill the feature.
    EdgePoly ep   = fp->getEdgePoly();
    gg->fillEdgePoly(ep, brush.color());

    // Outline the feature.
    gg->drawEdgePoly(ep,pen.color(), pen.width());
}

void  ViewerBase ::drawFigure(GeoGraphics * gg, FigurePtr fig, QPen pen)
{
    MapPtr map = fig->getFigureMap();
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
