/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include "tapp/RadialFigure.h"
#include "tile/Feature.h"
#include "geometry/Map.h"
#include "base/configuration.h"
#include "base/utilities.h"

RadialFigure::RadialFigure(const Figure & fig, int n, qreal rotate)
    : Figure(fig)
{
    this->n = n;
    r       = rotate;

    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);
    setFigType(FIG_TYPE_RADIAL);

    unitMap = make_shared<Map>();
}

RadialFigure::RadialFigure( int n, qreal rotate) : Figure()
{
    this->n = n;
    r       = rotate;

    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);
    setFigType(FIG_TYPE_RADIAL);

    unitMap = make_shared<Map>();
}

void RadialFigure::resetMaps()
{
    if (unitMap)
        unitMap->wipeout();
    Figure::resetMaps();
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
    if (config->debugReplicate)
    {
        // for debug don't replicate
        unitMap->duplicate(figureMap);    // contents are now the same
    }

#if 0
    figureMap = replicateUnit();
    //figureMap->dump();
#else
    //qDebug().noquote()  << "Tr=" << Tr.toString();

    figureMap->wipeout();

    QVector<QTransform> transforms;
    QTransform base = Tr;
    for( int idx = 0; idx < n; ++idx )
    {
        transforms.push_back(base);
        //base = base.compose(Tr);
        base = Tr * base;   // TODO xformo rder
    }

    unitMap->verify("RadialFigure::getMap-unit",false);

    figureMap->mergeSimpleMany(unitMap, transforms);

    figureMap->verify("RadialFigure::getMap-newret",false);

    //ret->dump();
#endif
}

MapPtr  RadialFigure::replicateUnit()
{
    qDebug() << "RadialFigure::replicateUnit";
    // DAC replicate the radial using N (not number of feature sides)
    QTransform T = Tr;       // rotaional transform
    MapPtr map = make_shared<Map>();
    for( int idx = 0; idx < getN(); ++idx )
    {
        for(auto e = unitMap->getEdges()->begin(); e != unitMap->getEdges()->end(); e++)
        {
            EdgePtr edge = *e;
            QPointF v1 = T.map( edge->getV1()->getPosition() );
            QPointF v2 = T.map( edge->getV2()->getPosition() );
            VertexPtr vp1 = map->insertVertex(v1);
            VertexPtr vp2 = map->insertVertex(v2);
            map->insertEdge(vp1,vp2);
        }
        //T.composeD(Tr);
        T *= Tr;    // TODO xform order
    }
    map->verify("Replicated Unit Map",true,true,true);
    return map;
}

void RadialFigure::setN(int n)
{
    this->n = n;
    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians( 2.0 * M_PI * don );
}

int RadialFigure::getN()
{
    return n;
}

void RadialFigure::setR( qreal rotate)
{
    r = rotate;
}

void RadialFigure::buildExtBoundary()
{
    Figure::buildExtBoundary();

    QTransform figureTransform;
    qreal scale = getFigureScale();
    figureTransform.scale(scale,scale);
    Feature f(getN());
    QPolygonF p = f.getPoints();
    setRadialFigBoundary(figureTransform.map(p));
    //qDebug() << "Fig boundary:" << figBoundary;
}
