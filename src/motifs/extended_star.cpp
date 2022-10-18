#include <QDebug>

#include "motifs/extended_star.h"

ExtendedStar::ExtendedStar(const Motif & fig, int nn, qreal d, int s) : Star(fig, nn, d, s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

ExtendedStar::ExtendedStar(int nn, qreal d, int s) : Star(nn,d,s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_STAR);
}

ExtendedStar::ExtendedStar(const ExtendedStar & other) : Star(other)
{
    extender = other.extender;
}

void ExtendedStar::buildMaps()
{
    Star::buildMaps();

    RadialMotif * rf = dynamic_cast<RadialMotif*>(this);
    QTransform Tr = rf->getTransform();

    Motif * fig = dynamic_cast<Motif*>(this);
    extender.extend(fig,Tr);
}

