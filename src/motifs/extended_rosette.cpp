#include "motifs/extended_rosette.h"

ExtendedRosette::ExtendedRosette(const Motif & fig, int nsides, qreal q, int s) : Rosette(fig, nsides,q,s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE);
}

ExtendedRosette::ExtendedRosette(int nsides, qreal q, int s) : Rosette(nsides,q,s)
{
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE);
}

ExtendedRosette::ExtendedRosette(const ExtendedRosette & other) : Rosette(other)
{
    extender = other.extender;
}

void ExtendedRosette::buildMotifMaps()
{
    Rosette::buildMotifMaps();
    
    extender.extend(this,getUnitRotationTransform());
}


