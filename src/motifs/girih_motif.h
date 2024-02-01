#pragma once
#ifndef GIRIH_MOTIF_H
#define GIRIH_MOTIF_H
#include "motifs/irregular_girih_branches.h"

class GirihMotif : public IrregularGirihBranches
{
public:
    GirihMotif();
    GirihMotif(const Motif & motif);

    void            init(qreal starSkip);

    virtual void    buildMotifMaps() override;
    virtual QString getMotifDesc()   override { return "GirihMotif"; }
    virtual void    report()         override { qDebug().noquote() << getMotifDesc() << "skip" << skip; }

protected:
    void            inferGirih();        // Girih inferring
};

#endif // GIRIH_MOTIF_H
