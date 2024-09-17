#pragma once
#ifndef GIRIH_MOTIF_H
#define GIRIH_MOTIF_H
#include "model/motifs/irregular_girih_branches.h"

class GirihMotif : public IrregularGirihBranches
{
public:
    GirihMotif();
    GirihMotif(const Motif & motif);

    void            init(qreal starSkip);

    virtual QString getMotifDesc()   override { return "GirihMotif"; }
    virtual void    dump()         override { qDebug().noquote() << getMotifDesc() << "skip" << skip; }

protected:
    void            infer() override;        // Girih inferring
};

#endif // GIRIH_MOTIF_H
