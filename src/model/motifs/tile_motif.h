#pragma once
#ifndef TILEMOTIF_H
#define TILEMOTIF_H


#include "model/motifs/irregular_motif.h"

class TileMotif  : public IrregularMotif
{
public:
    TileMotif();
    TileMotif(const Motif & other);

    virtual QString  getMotifDesc()   override { return "TileMotif"; }
    virtual void     dump()         override { qDebug().noquote() << getMotifDesc(); }

protected:
    void    infer() override; // make a motif from the tiling itself
};

#endif // TILEMOTIF_H
