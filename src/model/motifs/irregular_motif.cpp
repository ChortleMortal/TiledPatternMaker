#include <QDebug>
#include "model/motifs/irregular_motif.h"
#include "sys/geometry/map.h"
#include "sys/geometry/transform.h"
#include "model/tilings/tile.h"

// this is protected method used in class hierarchy
IrregularMotif::IrregularMotif() : Motif()
{
    init();
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
}

IrregularMotif::IrregularMotif(const IrregularMotif & other) : Motif(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
    skip        = other.skip;
    d           = other.d;
    s           = other.s;
    q           = other.q;
    r           = other.r;
    progressive = other.progressive;
}

IrregularMotif::IrregularMotif(const Motif & other) : Motif(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
    init();
}

IrregularMotif::IrregularMotif(MotifPtr other) : Motif(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
    if (other->isIrregular())
    {
        try
        {
            auto e_other = std::dynamic_pointer_cast<IrregularMotif>(other);
            skip        = e_other->skip;
            d           = e_other->d;
            s           = e_other->s;
            q           = e_other->q;
            r           = e_other->r;
            progressive = e_other->progressive;
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
}

void IrregularMotif::init()
{
    skip  = 3.0;         // girih
    d     = 2.0;         // hourglass + intersect + star
    s     = 1;           // hourglass + intersect + star
    q     = 0.0;         // rosette
    r     = 0.5;         // rosette
    progressive = false; // intersect
}

bool IrregularMotif::equals(const MotifPtr other)
{
    auto otherp = std::dynamic_pointer_cast<IrregularMotif>(other);
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

    if (progressive != otherp->progressive)
        return  false;

    if (!Motif::equals(other))
        return false;

     return true;
}

MapPtr IrregularMotif::getMotifMap()
{
    if (!motifMap || motifMap->isEmpty())
    {
        buildMotifMap();
    }
    return motifMap;
}

void IrregularMotif::buildMotifMap()
{
    Q_ASSERT(getTile());
    infer();
    if (motifMap)
    {
        scaleAndRotate(motifMap);
        completeMap();
        buildMotifBoundary();
        extendMaps();
    }
}

void IrregularMotif::extendMaps()
{
    for (ExtenderPtr extender : getExtenders())
    {
        extender->extendMotifMap();
    }
}

QTransform IrregularMotif::getMotifTransform()
{
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

void IrregularMotif::completeMap()
{
    motifMap->resetNeighbourMap();
    motifMap->verifyAndFix();
}

void IrregularMotif::resetMotifMap()
{
    motifMap.reset();
}

void IrregularMotif::dump()
{
    qDebug() << getMotifDesc() << "skip:" << skip << "d:" <<d << "s:" << s
             << "q:" << q  << "r (flexPt)" << r << ((progressive) ? "progressive" : "");
}

//////////////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////////////

IrregularNoMap::IrregularNoMap() : IrregularMotif()
{
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
}

IrregularNoMap::IrregularNoMap(const Motif & other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
}
