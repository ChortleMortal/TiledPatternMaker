#include <QDebug>
#include "model/motifs/adaptive_motif.h"
#include "model/motifs/adaptive_extender.h"
#include "model/tilings/tile.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/transform.h"

// this is protected method used in class hierarchy
AdaptiveMotif::AdaptiveMotif() : Motif()
{
    init();
    setMotifType(Type::NoMapMotif);
}

AdaptiveMotif::AdaptiveMotif(const AdaptiveMotif & other) : Motif(other)
{
    setMotifType(Type::NoMapMotif);
    skip        = other.skip;
    d           = other.d;
    s           = other.s;
    q           = other.q;
    r           = other.r;
    k           = other.k;
    progressive = other.progressive;
    cropToTile  = other.cropToTile;

    for (const ExtenderPtr & extender : other.get_extenders())
    {
        ExtenderPtr ep = std::make_shared<AdaptiveExtender>(*extender.get());
        ep->reparent(this);
        _extenders.append(ep);
    }
}

AdaptiveMotif::AdaptiveMotif(const Motif & other) : Motif(other)
{
    setMotifType(Type::NoMapMotif);
    if (other.isIrregular())
    {
        try
        {
            const AdaptiveMotif & e_other = static_cast<const AdaptiveMotif &>(other);
            skip        = e_other.skip;
            d           = e_other.d;
            s           = e_other.s;
            q           = e_other.q;
            r           = e_other.r;
            k           = e_other.k;
            progressive = e_other.progressive;
            cropToTile  = e_other.cropToTile;
        }
        catch(std::bad_cast &)
        {
            qWarning() << "Bad cast in Irregular Motif";
            init();
        }
    }
    else
    {
        init();
    }

    for (const ExtenderPtr & extender : other.get_extenders())
    {
        ExtenderPtr ep = std::make_shared<AdaptiveExtender>(*extender.get());
        ep->reparent(this);
        _extenders.append(ep);
    }
}

void AdaptiveMotif::init()
{
    skip  = 3.0;         // girih
    d     = 2.0;         // hourglass + intersect + star
    s     = 1;           // hourglass + intersect + star
    q     = 0.0;         // rosette
    r     = 0.5;         // rosette
    k     = 0.0;         // rosette
    progressive = false; // intersect
    cropToTile  = false;
}

bool AdaptiveMotif::equals(const MotifPtr other)
{
    auto otherp = std::dynamic_pointer_cast<AdaptiveMotif>(other);
    if (!otherp)
        return  false;

    if (getMotifType() != other->getMotifType())
        return false;

    if (d != otherp->d)
        return  false;

    if (s != otherp->s)
        return false;

    if (q != otherp->q)
        return  false;

    if (r != otherp->r)
        return  false;

    if (k != otherp->k)
        return  false;

    if (progressive != otherp->progressive)
        return  false;

    if (cropToTile != otherp->cropToTile)
        return  false;

    if (!Motif::equals(other))
        return false;

     return true;
}

void AdaptiveMotif::build()
{
    Q_ASSERT(getTile());

    infer();

    if (!hasValidMotifMap())
    {
        qWarning() << "AdaptiveMotif::build : invalid map";
        return;
    }

    irr_scaleAndRotate();
    irr_completeMap();
    irr_buildMotifBoundary();

    if (cropToTile)
    {
        motifMap->cropOutside3(getMotifBoundary());
        if (motifMap->isEmpty())
        {
            qWarning() << "AdaptiveMotif::build : motif map is empty after crop, skipping extend";
            return;
        }
    }

    irr_extendMaps();

    cleanseMotifMap();
}

ExtenderPtr AdaptiveMotif::createExtender()
{
    ExtenderPtr extender = std::make_shared<AdaptiveExtender>(this);
    return extender;
}

void AdaptiveMotif::irr_extendMaps()
{
    for (ExtenderPtr & extender : getExtenders())
    {
        extender->extend();
    }
}

void AdaptiveMotif::irr_scaleAndRotate()
{
    QTransform t = getMotifTransform();

    if (!t.isIdentity())
    {
        motifMap->transform(t);
    }
}

void AdaptiveMotif::irr_buildMotifBoundary()
{
    Q_ASSERT(getTile());

    QPolygonF boundary = getTile()->getPoints();

    if (!boundary.isClosed())
    {
        boundary << boundary[0];
    }

    QTransform t = getMotifTransform();
    boundary = t.map(boundary);
    setMotifBoundary(boundary);
}

QTransform AdaptiveMotif::getMotifTransform()
{
    Q_ASSERT(getTile());

    QPointF pt  = getTile()->getCenter();

    QTransform t;
    if (!Loose::equals(motifScale,1.0))
    {
        t *= Transform::scaleAroundPoint(pt,motifScale);
    }
    if (!Loose::zero(motifRotate))
    {
        t *= Transform::rotateDegreesAroundPoint(pt,motifRotate);
    }
    return t;
}

void AdaptiveMotif::irr_completeMap()
{
    MapVerifier mv(motifMap);
    mv.verifyAndFix();
}

void AdaptiveMotif::dump()
{
    qDebug() << getMotifDesc() << "skip:" << skip << "d:" <<d << "s:" << s
             << "q:" << q  << "r (flexPt)" << r << "k:" << k << ((progressive) ? "progressive" : "");
}

//////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////

IrregularNoMap::IrregularNoMap() : AdaptiveMotif()
{
    setMotifType(Type::NoMapMotif);
}

IrregularNoMap::IrregularNoMap(const Motif & other) : AdaptiveMotif(other)
{
    setMotifType(Type::NoMapMotif);
}
