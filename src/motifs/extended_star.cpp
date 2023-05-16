#include <QDebug>

#include "motifs/extended_star.h"

ExtendedStar::ExtendedStar(int nn, qreal d, int s) : Star(nn,d,s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

ExtendedStar::ExtendedStar(const Motif & fig, int nn, qreal d, int s) : Star(fig, nn, d, s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

ExtendedStar::ExtendedStar(const ExtendedStar & other) : Star(other)
{
    extender = other.extender;
}

void ExtendedStar::buildMotifMaps()
{
    Star::buildMotifMaps();

    extender.extend(this,getTransform());
}

