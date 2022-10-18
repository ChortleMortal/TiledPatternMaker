#include "motifs/extended_rosette.h"

ExtendedRosette::ExtendedRosette(const Motif & fig, int nsides, qreal q, int s, qreal k)
    : Rosette(fig, nsides,q,s,k)
{
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE);
}

ExtendedRosette::ExtendedRosette(int nsides, qreal q, int s, qreal k)
    : Rosette(nsides,q,s,k)
{
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE);
}

ExtendedRosette::ExtendedRosette(const ExtendedRosette & other) : Rosette(other)
{
    extender = other.extender;
}

void ExtendedRosette::buildMaps()
{
    Rosette::buildMaps();

    RadialMotif * rf = dynamic_cast<RadialMotif*>(this);
    QTransform Tr = rf->getTransform();

    Motif * fig = dynamic_cast<Motif*>(this);
    extender.extend(fig,Tr);
}


