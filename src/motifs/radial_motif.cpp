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
    _n = n;
    setupRadialTransform();
    setMotifType(MOTIF_TYPE_RADIAL);
}

RadialMotif::RadialMotif(int n) : Motif()
{
    _n = n;
    setupRadialTransform();
    setMotifType(MOTIF_TYPE_RADIAL);
}

RadialMotif::RadialMotif(const RadialMotif & motif) : Motif(motif)
{
    setupRadialTransform();

    if (motif.unitMap)
    {
        unitMap = motif.unitMap->copy();
    }
}

void RadialMotif::setN(int n)
{
    _n = n;
    setupRadialTransform();
}

// Get the point frac of the way around the unit circle.
QPointF RadialMotif::getArc( qreal frac )
{
    qreal ang = frac * 2.0 * M_PI;
    return QPointF( qCos( ang ), qSin( ang ) );
}

void RadialMotif::resetMotifMaps()
{
    unitMap.reset();
    motifMap.reset();
    debugMap.reset();
}

// Get a complete map from unit.
MapPtr RadialMotif::getMotifMap()
{
    //qDebug() << "RadialMotif::getMap";
    if (!unitMap)
    {
        buildMotifMaps();
    }
    return motifMap;
}

void RadialMotif::buildMotifMaps()
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

void RadialMotif::setupRadialTransform()
{
    don     = 1.0 / qreal(getN());
    Tr      = QTransform().rotateRadians( 2.0 * M_PI * don );
}

void RadialMotif::buildRadialBoundaries()
{
    // Build Extended boundaary
    extendedBoundary.buildRadial();

    // build figure boundary
    if (getN() >=3)
    {
        Tile f(getN(),getMotifRotate(), getMotifScale());
        QPolygonF p = f.getPolygon();
        setMotifBoundary(p);
    }
}
