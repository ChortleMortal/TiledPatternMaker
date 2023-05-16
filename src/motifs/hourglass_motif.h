#pragma once
#ifndef HOURGLASS_MOTIF_H
#define HOURGLASS_MOTIF_H

#include "motifs/irregular_star_branches.h"

class HourglassMotif : public IrregularStarBranches
{
public:
    HourglassMotif();
    HourglassMotif(const Motif &other);

    void init(qreal d, int s);

    virtual void    buildMotifMaps()  override;
    virtual QString getMotifDesc()    override { return "HourglassMotif"; }
    virtual void    report()          override { qDebug().noquote() << getMotifDesc() << "d" << d << "s" << s; }

protected:
    void inferHourglass(TilePtr tile);                       // Hourglass inferring
};

#endif // HOURGLASS_MOTIF_H
