#pragma once

#include "model/motifs/adaptive_girih_branches.h"

class AdaptiveGirih : public AdaptiveGirihBranches
{
public:
    AdaptiveGirih();
    AdaptiveGirih(const Motif & motif);

    void            init(qreal starSkip);

    virtual QString getMotifDesc() override { return "AdaptiveGirih"; }
    virtual void    dump()         override { qDebug().noquote() << getMotifDesc() << "skip" << skip; }
    virtual int     maxVersions()  override { return 1; }

protected:
    void            infer() override;        // Girih inferring
};
