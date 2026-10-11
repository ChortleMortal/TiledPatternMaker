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
#include "model/motifs/radial_extender.h"
#include "model/motifs/radial_motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/map.h"
#include "sys/sys/debugflags.h"

using std::make_shared;

RadialMotif::RadialMotif(int n) : Motif()
{
    d        = 3.0;
    s        = 2;
    q        = 0.0;
    kneeX    = 0.25;
    kneeY    = 0.25;
    k        = 0.0;
    inscribe = false;
    onPoint  = false;

    setN(n);
    setMotifType(Type::Radial);
}

RadialMotif::RadialMotif(const Motif & motif) : Motif(motif)
{
    if (motif.isRadial())
    {
        const RadialMotif & radial = static_cast<const RadialMotif &>(motif);
        d        = radial.d;
        s        = radial.s;
        q        = radial.q;
        kneeX    = radial.kneeX;
        kneeY    = radial.kneeY;
        k        = radial.k;
        inscribe = radial.inscribe;
        onPoint  = radial.onPoint;
    }
    else
    {
        d        = 3.0;
        s        = 2;
        q        = 0.0;
        kneeX    = 0.25;
        kneeY    = 0.25;
        k        = 0.0;
        inscribe = false;
        onPoint  = false;
    }

    setN(motif.getN());

    for (const ExtenderPtr & extender : motif.get_extenders())
    {
        ExtenderPtr ep = make_shared<RadialExtender>(*extender.get());
        ep->reparent(this);
        _extenders.append(ep);
    }

    setMotifType(Type::Radial);
}

RadialMotif::RadialMotif(const RadialMotif & radial) : Motif(radial)
{
    d        = radial.d;
    s        = radial.s;
    q        = radial.q;
    kneeX    = radial.kneeX;
    kneeY    = radial.kneeY;
    k        = radial.k;
    onPoint  = radial.onPoint;
    inscribe = radial.inscribe;

    set_don(1.0 / qreal(getN()));

    for (const ExtenderPtr & extender : radial.get_extenders())
    {
        ExtenderPtr ep = make_shared<RadialExtender>(*extender.get());
        ep->reparent(this);
        _extenders.append(ep);
    }
}

void RadialMotif::setN(int n)
{
    Motif::setN(n);
    set_don(1.0 / qreal(getN()));
}

void RadialMotif::buildFullRays()
{
    if (Sys::flags->flagged(SHOW_UNIT_RAYS))
    {
        qDebug() << "unit rays size" << unitRays.size();
        showRays(unitRays);
    }

    if (Sys::dontReplicate)
    {
        fullRays = unitRays;
    }
    else
    {
        replicateRays(); // replicate from unitRay to  fullRay
    }
    fullRays.cadencePairs   = unitRays.size() / 2;
    fullRays.unextendedSize = fullRays.size();
    Q_ASSERT(fullRays.valid());
    //fullRays.debug(1);

    if (Sys::flags->flagged(SHOW_FULL_RAYS))
    {
        showRays(fullRays);
    }
}

void RadialMotif::extend()
{
    for (ExtenderPtr & extender : getExtenders())
    {
        extender->extend();
    }
}

void RadialMotif::embellish()
{
}

ExtenderPtr RadialMotif::createExtender()
{
    ExtenderPtr extender = make_shared<RadialExtender>(this);
    return extender;
}

qreal RadialMotif::getMotifFitScale()
{
    for (ExtenderPtr & extender : getExtenders())
    {
        if (extender->getStellation() > 0)
        {
            auto radialExt = static_pointer_cast<RadialExtender>(extender);
            return radialExt->getFitScale();
        }
    }
    return 1.0;
}

// Get the point frac of the way around the unit circle.
QPointF RadialMotif::getArc( qreal frac )
{
    qreal ang = frac * 2.0 * M_PI;
    return QPointF( qCos( ang ), qSin( ang ) );
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

void RadialMotif::buildMap()
{
    // build motifMap from RaySet
    motifMap = make_shared<Map>("Radial replicated unit map");
    for (auto & ray : fullRays.get())
    {
        ray.addToMap(motifMap);
    }

    for (ExtenderPtr extender : getExtenders())
    {
        if (extender->_embedBoundary)
        {
            extender->embedBoundaryInMap(motifMap);
        }
        if (extender->_embedTile)
        {
            extender->embedTileInMap(motifMap,getTile());
        }
    }

    cleanseMotifMap();
}

void RadialMotif::build()
{
    //qInfo() << "** building motif:" << getMotifDesc() << getN();

    Sys::debugMapMotif->wipeout();

    unitRays.clear();

    buildUnitRays();    // unit rays are scaled and rotated

    buildFullRays();    // full rays are replicated

    embellish();        // a placeholder - does nothing fornow

    extend();           // aplly extentions and stellation

    buildMap();         // creates motif map
}

void RadialMotif::replicateRays()
{
    // Builds fullRays from unitRays

    // this algorith deals with a set of 2, 4, or 6 rays in unitRays
    // this is applied to a motif of size N to form fullRays

    fullRays.clear();

    int rsize = unitRays.size();
    if (rsize == 0)
    {
        qWarning() << "Empty Unit Rays";
        return;
    }

    int cadence = unitRays.size() / 2;      // LR pairs
    int side = 0;
    while (side < getN())
    {
        RaySet unitCopy = unitRays;
        QTransform t = getRadialTransform(side);
        unitCopy.transform(t);

        for (int lr = 0; lr < cadence; lr++)
        {
            int idx = lr * 2;  // each 'arm' has rwo rays
            RadialRay rr = unitCopy[idx % rsize];
            //rr.transform(getRadialTransform(idx));
            fullRays.add(rr);

            rr = unitCopy[(idx +1) % rsize];
            //rr.transform(getRadialTransform(idx));
            fullRays.add(rr);
        }
        side += cadence;
    }
    //qDebug() << "fullRays" << fullRays.size();
}

QTransform RadialMotif::getRadialTransform(int side)
{
    if (side ==0)
        return QTransform();

    return QTransform().rotateRadians(2 * M_PI * don() * (qreal)side);
}

QTransform RadialMotif::getMotifTransform()
{
    Q_ASSERT(getTile());

    QTransform tmotif = QTransform::fromScale(motifScale,motifScale).rotate(motifRotate);

    return getTile()->getEpolyTransform() * tmotif;
}

void RadialMotif::showRays(RaySet & rays)
{
    // paint unit rays
    int count = 0;
    for (int i = 0; i < rays.size(); i+=2)
    {
        auto t = getRadialTransform(count++);

        auto ray = rays[i];
        for (int j=0; j < ray.size(); j++)
        {
            auto pt = ray.pt(j);
            pt = t.map(pt);
            QString astring = QString("[%1][%2]").arg(i).arg(j);
            Sys::debugMapMotif->insertDebugMark(pt,astring);
        }

        ray = rays[i+1];
        for (int j=0; j < ray.size(); j++)
        {
            auto pt = ray.pt(j);
            pt = t.map(pt);
            QString astring = QString("[%1][%2]").arg(i+1).arg(j);
            Sys::debugMapMotif->insertDebugMark(pt,astring);
        }
    }
}
