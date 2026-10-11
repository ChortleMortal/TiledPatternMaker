#pragma once

#include "model/motifs/adaptive_motif.h"

class AdaptiveTile  : public AdaptiveMotif
{
public:
    AdaptiveTile();
    AdaptiveTile(const Motif & other);

    virtual QString  getMotifDesc()   override { return "AdattiveTile"; }
    virtual void     dump()           override { qDebug().noquote() << getMotifDesc(); }
    virtual int      maxVersions()    override { return 1; }

protected:
    void    infer() override; // make a motif from the tiling itself
};
