#pragma once

#include "model/motifs/adaptive_star_branches.h"

class AdaptiveHourglass : public AdaptiveStarBranches
{
public:
    AdaptiveHourglass();
    AdaptiveHourglass(const Motif &other);

    void init(qreal d, int s);

    virtual QString getMotifDesc()  override { return "AdaptiveHourglass"; }
    virtual void    dump()          override { qDebug().noquote() << getMotifDesc() << "d" << d << "s" << s; }
    virtual int     maxVersions()   override { return 1; }

protected:
    void infer() override; // Hourglass inferring
};
