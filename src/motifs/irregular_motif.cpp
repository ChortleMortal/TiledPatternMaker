#include <QDebug>
#include "motifs/irregular_motif.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "tile/tile.h"

// this is protected method used in class hierarchy
IrregularMotif::IrregularMotif() : Motif()
{
    init();
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
}

IrregularMotif::IrregularMotif(const Motif & other) : Motif(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_NO_MAP);
    if (other.isIrregular())
    {
        try
        {
            auto e_other = dynamic_cast<const IrregularMotif&>(other);
            skip        = e_other.skip;
            d           = e_other.d;
            s           = e_other.s;
            q           = e_other.q;
            r           = e_other.r;
            progressive = e_other.progressive;
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
#ifdef BUILD_ON_DEMAND
    if (!motifMap || motifMap->isEmpty())
    {
        buildMotifMaps();
    }
#endif
    return motifMap;
}

void IrregularMotif::buildMotifMaps()
{
    // there is no map to build
    Q_ASSERT(getTile());
    buildMotifBoundary();
    buildExtendedBoundary();
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

void IrregularMotif::resetMotifMaps()
{
    motifMap.reset();
}

void IrregularMotif::dump()
{
    qDebug() << "IrregularMotif - skip:" << skip << "d:" <<d << "s:" << s
             << "q:" << q  << "r (flexPt)" << r << ((progressive) ? "progressive" : "");
}

