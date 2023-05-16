#pragma once
#ifndef TILEMOTIF_H
#define TILEMOTIF_H


#include "motifs/irregular_motif.h"

class TileMotif  : public IrregularMotif
{
public:
    TileMotif();
    TileMotif(const Motif & other);

    virtual void     buildMotifMaps() override;

    virtual QString  getMotifDesc()   override { return "TileMotif"; }
    virtual void     report()         override { qDebug().noquote() << getMotifDesc(); }

protected:
    void    inferTile(TilePtr tile); // make a motif from the tiling itself
};

#endif // TILEMOTIF_H
