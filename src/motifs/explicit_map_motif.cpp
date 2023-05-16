#include "motifs/explicit_map_motif.h"
#include "geometry/map.h"

ExplicitMapMotif::ExplicitMapMotif() : IrregularMotif()
{
    setMotifType(MOTIF_TYPE_EXPLICIT_MAP);
    explicitMap = std::make_shared<Map>("Dummy explicit map");
};

ExplicitMapMotif::ExplicitMapMotif(MapPtr map) : IrregularMotif()
{
    setMotifType(MOTIF_TYPE_EXPLICIT_MAP);
    explicitMap = map;
}

ExplicitMapMotif::ExplicitMapMotif(const Motif &other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_EXPLICIT_MAP);
    explicitMap = other.motifMap;
}

void ExplicitMapMotif::buildMotifMaps()
{
    Q_ASSERT(tile);
    Q_ASSERT(explicitMap);
    motifMap = completeMap(explicitMap);
}

void ExplicitMapMotif::resetMotifMaps()
{
    explicitMap.reset();
    motifMap.reset();
}
