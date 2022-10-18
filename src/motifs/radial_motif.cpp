////////////////////////////////////////////////////////////////////////////
//
// RadialFigure.java
//
// A RadialMotif is a special kind of Motif that has d_n symmetry.  That
// means that it can be rotated by 360/n degrees and flipped across certain
// lines through the origin and it looks the same.
//
// We take advantage of this by only making subclasses produce a basic
// unit, i.e. a smaller map that generates the complete figure through the
// action of c_n (just the rotations; the reflections are factored in
// by subclasses).

#include <QtMath>
#include <QDebug>
#include "motifs/radial_motif.h"
#include "tile/tile.h"
#include "geometry/map.h"
#include "settings/configuration.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

using std::make_shared;

RadialMotif::RadialMotif(const Motif & motif, int n) : Motif(motif)
{
    this->n = n;

    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);
    setMotifType(MOTIF_TYPE_RADIAL);

    //unitMap = make_shared<Map>("radial unit map1");
}

RadialMotif::RadialMotif(int n) : Motif()
{
    this->n = n;

    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);
    setMotifType(MOTIF_TYPE_RADIAL);

    //unitMap = make_shared<Map>("raidal unit map2");
}

RadialMotif::RadialMotif(const RadialMotif & motif) : Motif(motif)
{
    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians(2.0 * M_PI * don);

    if (motif.unitMap)
    {
        unitMap = motif.unitMap->copy();
    }
}

// Get the point frac of the way around the unit circle.
QPointF RadialMotif::getArc( qreal frac )
{
    qreal ang = frac * 2.0 * M_PI;
    return QPointF( qCos( ang ), qSin( ang ) );
}

void RadialMotif::resetMaps()
{
    unitMap.reset();
    motifMap.reset();
    debugMap.reset();
}

// Get a complete map from unit.
MapPtr RadialMotif::getMap()
{
    //qDebug() << "RadialMotif::getMap";
    if (!unitMap)
    {
        buildMaps();
    }
    return motifMap;
}

void RadialMotif::buildMaps()
{
    buildUnitMap();

    Configuration * config = Configuration::getInstance();
    if (config->dontReplicate)
    {
        motifMap = unitMap;    // contents are now the same
    }
    else
    {
        replicate();
    }
}

void RadialMotif::replicate()
{
    // DAC - replicate the radial using N (not number of tile sides)
    qDebug() << "RadialMotif::replicateUnit";
    QTransform T = Tr;       // rotaional transform
    motifMap = make_shared<Map>("radial replicated unit map");
    for( int idx = 0; idx < getN(); ++idx )
    {
        for (auto & edge : qAsConst(unitMap->getEdges()))
        {
            QPointF v1 = T.map(edge->v1->pt);
            QPointF v2 = T.map(edge->v2->pt);
            VertexPtr vp1 = motifMap->insertVertex(v1);
            VertexPtr vp2 = motifMap->insertVertex(v2);
            motifMap->insertEdge(vp1,vp2);
        }
        T *= Tr;
    }

    motifMap->verify();
}

void RadialMotif::setN(int n)
{
    this->n = n;
    dn      = qreal(n);
    don     = 1.0 / dn;
    Tr      = QTransform().rotateRadians( 2.0 * M_PI * don );
}

void RadialMotif::buildRadialBoundaries()
{
    // Build Extended boundaary
    buildExtendedRadialBoundary();

    // build figure boundary
    Tile f(getN(),getMotifRotate(), getMotifScale());
    QPolygonF p = f.getPolygon();
    setMotifBoundary(p);
}
