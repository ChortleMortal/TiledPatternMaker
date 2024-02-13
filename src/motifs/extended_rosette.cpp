#include "motifs/extended_rosette.h"

/////////////////////////////////////////
/// ExtendedRosette
////////////////////////////////////////

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
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE);
}

void ExtendedRosette::buildMotifMaps()
{
    Rosette::buildMotifMaps();
    
    extender.extend(this,getUnitRotationTransform());
}


/////////////////////////////////////////
/// ExtendedRosette2
////////////////////////////////////////

ExtendedRosette2::ExtendedRosette2(const Motif & fig, int n, qreal kneeX, qreal kneeY, int s, qreal k, bool c) : Rosette2(fig,n,kneeX,kneeY,s,k,c)
{
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE2);
}

ExtendedRosette2::ExtendedRosette2(int n, qreal kneeX, qreal kneeY, int s, qreal k, bool c) : Rosette2(n,kneeX,kneeY,s, k, c)
{
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE2);
}

ExtendedRosette2::ExtendedRosette2(const ExtendedRosette2 & other) : Rosette2(other)
{
    extender = other.extender;
    setMotifType(MOTIF_TYPE_EXTENDED_ROSETTE2);
}

void ExtendedRosette2::buildMotifMaps()
{
    Rosette2::buildMotifMaps();

    extender.extend(this,getUnitRotationTransform());
}
