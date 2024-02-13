#include <QDebug>

#include "motifs/extended_star.h"

/////////////////////////////////////////
/// ExtendedStar
////////////////////////////////////////
ExtendedStar::ExtendedStar(int n, qreal d, int s) : Star(n,d,s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

ExtendedStar::ExtendedStar(const Motif & motif, int n, qreal d, int s) : Star(motif, n, d, s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

ExtendedStar::ExtendedStar(const ExtendedStar & other) : Star(other)
{
    extender = other.extender;
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

void ExtendedStar::buildMotifMaps()
{
    Star::buildMotifMaps();

    extender.extend(this,getUnitRotationTransform());
}

/////////////////////////////////////////
/// ExtendedStar2
////////////////////////////////////////
ExtendedStar2::ExtendedStar2(int n, qreal theta, int intersects) : Star2(n,theta,intersects)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR2);
}

ExtendedStar2::ExtendedStar2(const Motif & motif, int n, qreal theta, int intersects) : Star2(motif, n, theta, intersects)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR2);
}

ExtendedStar2::ExtendedStar2(const ExtendedStar2 &other) : Star2(other)
{
    extender = other.extender;
    setMotifType(MOTIF_TYPE_EXTENDED_STAR2);
}

void ExtendedStar2::buildMotifMaps()
{
    Star2::buildMotifMaps();
    
    extender.extend(this,getUnitRotationTransform());
}

