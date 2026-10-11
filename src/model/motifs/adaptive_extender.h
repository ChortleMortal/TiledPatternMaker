#pragma once

#include "model/motifs/extender.h"

class AdaptiveExtender : public Extender
{
public:
    AdaptiveExtender(Motif * motif);
    AdaptiveExtender(const Extender & other);

    void    extend() override final;

private:
    void    extend(QTransform Tr = QTransform());
    void    extendMotifRays(MapPtr motifMap);
    void    extendBoundaryMap(MapPtr motifMap, TilePtr tile);
    void    extendTipsToBoundary(MapPtr motifMap, QTransform unitRotationTr);
    void    connectOuterVertices(MapPtr motifMap);

    UniqueQVector<VertexPtr> newVertices;
    //TODO - experimentr: put scale here to shrink mosaic and allow it to be extended
};

