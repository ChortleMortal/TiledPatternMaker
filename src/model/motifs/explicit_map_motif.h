#pragma once

#include "model/motifs/adaptive_motif.h"

// This is the carrier of the historic motif maps from taprats
// You can make an explict motif from a regular or irregular motif
// But you cant make any other moptif from the explicit map
// Its data is carried in the explicit Map and it can be sclaed and rotated into the motif mao


class ExplicitMapMotif : public AdaptiveMotif
{
public:
    ExplicitMapMotif();
    ExplicitMapMotif(MapPtr map);
    ExplicitMapMotif(const Motif & other);

    void    infer() override;

    virtual QString getMotifDesc()  override { return "ExplicitMapMotif"; }
    virtual void    dump()          override { qDebug().noquote() << getMotifDesc(); }
    virtual int     maxVersions()   override { return 1; }

    MapPtr  newExplicitMap();
    void    setExplicitMap(MapPtr map);
    MapPtr  getExplicitMap() { return explicitMap; }

    virtual void    resetMotifMap() override {}
    virtual MapPtr  getMotifMap() override { return explicitMap; }

protected:

private:
    MapPtr  explicitMap;
};
