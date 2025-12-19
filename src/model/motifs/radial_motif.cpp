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
#include "model/motifs/radial_motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"
#include "sys/geometry/map_verifier.h"

using std::make_shared;

RadialMotif::RadialMotif(const Motif & motif, int n) : Motif(motif)
{
    Motif::setN(n);

    if (motif.isRadial())
    {
        const RadialMotif & rmp = static_cast<const RadialMotif &>(motif);
        inscribe = rmp.inscribe;
        onPoint  = rmp.onPoint;
    }
    else
    {
        inscribe = false;
        onPoint  = false;
    }
    setupRadialTransform();
    setMotifType(MOTIF_TYPE_RADIAL);
}

RadialMotif::RadialMotif(int n) : Motif()
{
    Motif::setN(n);
    inscribe = false;
    onPoint  = false;
    setupRadialTransform();
    setMotifType(MOTIF_TYPE_RADIAL);
}

RadialMotif::RadialMotif(const RadialMotif & motif) : Motif(motif)
{
    onPoint  = motif.onPoint;
    inscribe = motif.inscribe;

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

void RadialMotif::resetMotifMap()
{
    unitMap.reset();
    motifMap.reset();
}

// Get a complete map from unit.
MapPtr RadialMotif::getMotifMap()
{
    //qDebug() << "RadialMotif::getMap";
    if (!unitMap)
    {
        buildMotifMap();
    }
    return motifMap;
}

void  RadialMotif::setOnPoint(bool enb)
{
    onPoint = enb;
    resetMotifMap();
}

void RadialMotif::setInscribe(bool enb)
{
    inscribe = enb;
    resetMotifMap();
}

void RadialMotif::buildMotifMap()
{
    buildUnitMap();

    for (ExtenderPtr extender : getExtenders())
    {
        extender->buildExtendedBoundary();

        if (extender->_extendRays)
        {
            extender->extendRayToBoundary(raySet1.ray1);
            extender->extendRayToBoundary(raySet1.ray2);
            if (raySet2.valid())
            {
                extender->extendRayToBoundary(raySet2.ray1);
                extender->extendRayToBoundary(raySet2.ray2);
            }
        }

        if (extender->_extendTipsToBound)
        {
            extender->extendRayToBoundaryPerp(raySet1.ray1);
            extender->extendRayToBoundaryPerp(raySet1.ray2);
            if (raySet2.valid())
            {
                extender->extendRayToBoundaryPerp(raySet2.ray1);
                extender->extendRayToBoundaryPerp(raySet2.ray2);
            }
        }

        if (extender->_extendTipsToTile)
        {
            extender->extendRayToTilePerp(raySet1.ray1,getTile());
            extender->extendRayToTilePerp(raySet1.ray2,getTile());
            if (raySet2.valid())
            {
                extender->extendRayToTilePerp(raySet2.ray1,getTile());
                extender->extendRayToTilePerp(raySet2.ray2,getTile());
            }
        }

        uint rayConnectMothod = extender->_connectRays;
        if (rayConnectMothod > 0)  // connect to each other
        {
            extender->connectRays(rayConnectMothod,raySet1,raySet2);
        }
    }

    ConnectPtr connector = getRadialConnector();
    if (connector)
    {
        connector->build(this);
    }

    unitMap  = createUnitMapFromRays(raySet1);
    unitMap2 = createUnitMapFromRays(raySet2);

    if (Sys::dontReplicate)
    {
        motifMap = unitMap;
        motifMap->mergeMap(unitMap2);
    }
    else
    {
        replicate();
    }

    for (ExtenderPtr extender : getExtenders())
    {
        if (extender->_embedBoundary)
        {
            extender->embedBoundary(motifMap);
        }
        if (extender->_embedTile)
        {
            extender->embedTile(motifMap,getTile());
        }
    }

    if (cleanseVal > 0)
    {
        MapCleanser mc(motifMap);
        mc.cleanse(cleanseVal,sensitivity);
    }
}

MapPtr RadialMotif::createUnitMapFromRays(RaySet & set)
{
    if (motifDebug & 0x04) set.debug();

    MapPtr map;
    if (set.ray1.valid())
    {
        map = std::make_shared<Map>("UnitMap");
        set.ray1.addToMap(map);
        if (set.ray2.valid())
        {
            set.ray2.addToMap(map);
        }
    }
    return map;
}

void RadialMotif::replicate()
{
    QTransform T2 = radialRotationTr * radialRotationTr;       // rotaional transform

    motifMap = make_shared<Map>("Radial replicated unit map");

    for( int idx = 0; idx < getN(); idx++)
    {
        if (idx & 1)
        {
            motifMap->mergeMap(unitMap2);
            unitMap->transform(T2);
            unitMap2->transform(T2);
        }
        else
        {
            motifMap->mergeMap(unitMap);
        }
    }

    MapVerifier mv(motifMap);
    mv.verify();
}

void RadialMotif::setupRadialTransform()
{
    don               = 1.0 / qreal(getN());
    radialRotationTr  = QTransform().rotateRadians(2.0 * M_PI * don);
}

QTransform RadialMotif::getMotifTransform()
{
    return QTransform::fromScale(motifScale,motifScale).rotate(motifRotate);
}

QTransform RadialMotif::getDELTransform()
{
    Q_ASSERT(getTile());
    return getTile()->getTransform() * getMotifTransform();
}

qreal RadialMotif::computeConnectScale()
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.
    qreal scale = 1.0;
    if (connector)
    {
        scale  = connector->build(this);
    }
    return scale;
}
