#include "model/motifs/explicit_map_motif.h"
#include "sys/geometry/map.h"

ExplicitMapMotif::ExplicitMapMotif() : AdaptiveMotif()
{
    setMotifType(Type::MapMotif);
    explicitMap = std::make_shared<Map>("Dummy explicit map");
};

ExplicitMapMotif::ExplicitMapMotif(MapPtr map) : AdaptiveMotif()
{
    setMotifType(Type::MapMotif);
    explicitMap = map;
}

ExplicitMapMotif::ExplicitMapMotif(const Motif &other) : AdaptiveMotif(other)
{
    setMotifType(Type::MapMotif);
    explicitMap = other.motifMap;
}

void ExplicitMapMotif::infer()
{
    motifMap = explicitMap->recreate();
}
