////////////////////////////////////////////////////////////////////////////
//
// RadialFigure.java
//
// A RadialFigure is a special kind of Figure that has d_n symmetry.  That
// means that it can be rotated by 360/n degrees and flipped across certain
// lines through the origin and it looks the same.
//
// We take advantage of this by only making subclasses produce a basic
// unit, i.e. a smaller map that generates the complete figure through the
// action of c_n (just the rotations; the reflections are factored in
// by subclasses).

#include <QtMath>
#include <QDebug>
#include "figures/radial_figure.h"
#include "tile/feature.h"
#include "geometry/map.h"
#include "settings/configuration.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

using std::make_shared;

RadialFigure::RadialFigure(const Figure & fig, int n) : Figure(fig)
{
    this->n = n;

    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);
    setFigType(FIG_TYPE_RADIAL);

    unitMap = make_shared<Map>("radial unit map1");
}

RadialFigure::RadialFigure(int n) : Figure()
{
    this->n = n;

    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);
    setFigType(FIG_TYPE_RADIAL);

    unitMap = make_shared<Map>("raidal unit map2");
}

void RadialFigure::resetMaps()
{
    if (unitMap)
        unitMap->wipeout();
    if (figureMap)
        figureMap->wipeout();
    if (debugMap)
        debugMap->wipeout();
}

// Get the point frac of the way around the unit circle.
QPointF RadialFigure::getArc( qreal frac )
{
    qreal ang = frac * 2.0 * M_PI;
    return QPointF( qCos( ang ), qSin( ang ) );
}


// Get a complete map from unit.
MapPtr RadialFigure::getFigureMap()
{
    //qDebug() << "RadialFigure::getFigureMap";
    if (unitMap->isEmpty())
    {
        buildMaps();
    }
    return figureMap;
}

void RadialFigure::buildMaps()
{
    unitMap = buildUnit();

    Configuration * config = Configuration::getInstance();
    if (config->dontReplicate)
    {
        figureMap = unitMap;    // contents are now the same
    }
    else
    {
        figureMap = replicateUnit();
    }
}

MapPtr  RadialFigure::replicateUnit()
{
    qDebug() << "RadialFigure::replicateUnit";
    // DAC replicate the radial using N (not number of feature sides)
    QTransform T = Tr;       // rotaional transform
    MapPtr map = make_shared<Map>("radial replicated unit map");
    for( int idx = 0; idx < getN(); ++idx )
    {
        for (auto & edge : unitMap->getEdges())
        {
            QPointF v1 = T.map(edge->v1->pt);
            QPointF v2 = T.map(edge->v2->pt);
            VertexPtr vp1 = map->insertVertex(v1);
            VertexPtr vp2 = map->insertVertex(v2);
            map->insertEdge(vp1,vp2);
        }
        T *= Tr;
    }

    map->verify();

    return map;
}

void RadialFigure::setN(int n)
{
    this->n = n;
    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians( 2.0 * M_PI * don );
}

void RadialFigure::buildExtBoundary()
{
    Figure::buildExtBoundary();

    QTransform figureTransform;
    qreal scale = getFigureScale();
    figureTransform.scale(scale,scale);
    Feature f(getN(),0);
    QPolygonF p = f.getPoints();
    setRadialFigBoundary(figureTransform.map(p));
    //qDebug() << "Fig boundary:" << figBoundary;
}
