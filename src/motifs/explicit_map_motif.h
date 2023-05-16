#pragma once
#ifndef EXPLICIT_MAP_MOTIF_H
#define EXPLICIT_MAP_MOTIF_H

#include "motifs/irregular_motif.h"

// This is the carrier of the historic motif maps from taprats
// You can make an explict motif from a regular or irregular motif
// But you cant make any other moptif from the explicit map
// Its data is carried in the explicit Map and it can be sclaed and rotated into the motif mao


class ExplicitMapMotif : public IrregularMotif
{
public:
    ExplicitMapMotif();
    ExplicitMapMotif(MapPtr map);
    ExplicitMapMotif(const Motif & other);

    virtual void    resetMotifMaps()  override;
    void            buildMotifMaps()  override;
    virtual QString getMotifDesc()    override { return "ExplicitMapMotif"; }
    virtual void    report()          override { qDebug().noquote() << getMotifDesc(); }

    MapPtr          newExplicitMap();
    void            setExplicitMap(MapPtr map);
    MapPtr          getExplicitMap() { return explicitMap; }

protected:

private:
    MapPtr  explicitMap;
};

#endif
