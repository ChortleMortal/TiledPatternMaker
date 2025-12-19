#include "model/motifs/explicit_map_motif.h"
#include "sys/geometry/map.h"

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

void ExplicitMapMotif::infer()
{
    motifMap = explicitMap->recreate();
}

void ExplicitMapMotif::resetMotifMap()
{
    //explicitMap.reset();  // the explicit map should not be messed with
    motifMap.reset();
}
