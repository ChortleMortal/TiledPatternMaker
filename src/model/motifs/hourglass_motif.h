#pragma once
#ifndef HOURGLASS_MOTIF_H
#define HOURGLASS_MOTIF_H

#include "model/motifs/irregular_star_branches.h"

class HourglassMotif : public IrregularStarBranches
{
public:
    HourglassMotif();
    HourglassMotif(const Motif &other);

    void init(qreal d, int s);

    virtual QString getMotifDesc()    override { return "HourglassMotif"; }
    virtual void    dump()          override { qDebug().noquote() << getMotifDesc() << "d" << d << "s" << s; }

protected:
    void infer() override; // Hourglass inferring
};

#endif // HOURGLASS_MOTIF_H
