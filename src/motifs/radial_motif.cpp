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
#include "geometry/map.h"
#include "geometry/map.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"

using std::make_shared;

RadialMotif::RadialMotif(const Motif & motif, int n) : Motif(motif)
{
    Motif::setN(n);
    setupRadialTransform();
    setMotifType(MOTIF_TYPE_RADIAL);
}

RadialMotif::RadialMotif(int n) : Motif()
{
    Motif::setN(n);
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
    Motif::setN(n);
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

    if (Sys::dontReplicate)
    {
        motifMap = unitMap; // contents are now the same
    }
    else
    {
        replicate();        // creates motif map
    }
    
    scaleAndRotate();       // scales and rotates motif map
    buildMotifBoundary();
    buildExtendedBoundary();
}

void RadialMotif::replicate()
{
    // DAC - replicate the radial using N (not number of tile sides)
    //qDebug() << "RadialMotif::replicateUnit";
    QTransform T = radialRotationTr;       // rotaional transform
    motifMap = make_shared<Map>("radial replicated unit map");
    for( int idx = 0; idx < getN(); ++idx )
    {
        for (auto & edge : std::as_const(unitMap->getEdges()))
        {
            QPointF v1 = T.map(edge->v1->pt);
            QPointF v2 = T.map(edge->v2->pt);
            VertexPtr vp1 = motifMap->insertVertex(v1);
            VertexPtr vp2 = motifMap->insertVertex(v2);
            motifMap->insertEdge(vp1,vp2);
        }
        T *= radialRotationTr;
    }

    motifMap->verify();
}

void RadialMotif::setupRadialTransform()
{
    don               = 1.0 / qreal(getN());
    radialRotationTr  = QTransform().rotateRadians(2.0 * M_PI * don);
}
