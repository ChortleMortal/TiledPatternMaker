#pragma once

#include "model/motifs/extender.h"

class RadialMotif;

class RadialExtender : public Extender
{
public:
    RadialExtender(Motif * motif);
    RadialExtender(const Extender & other);

    void    extend() override final;

private:
    void    extend(RaySet & rays);
    void    extendRaysToBoundary(RaySet & rays);
    void    extendRaysToBoundaryPerp(RaySet & rays);
    void    extendRaysToTilePerp(RaySet & rays, TilePtr tile);
    bool    tipMoves(RaySet & rays, int i);

    void    extendRays(uint method, RaySet & rays);
    void    stellate(RadialMotif * motif, uint stellation, bool fitTile);

    void    addExtenderRaysToBoundary(RaySet & rays);
    RadialRay addExtenderRayToBoundary(RadialRay & ray, EdgePoly & boundary);

    void    joinRays(RaySet & rays, int rayIndex, int ptIndex);
    void    intersectRays(RaySet & rays, int rayIndex, int ptIndex);
};

