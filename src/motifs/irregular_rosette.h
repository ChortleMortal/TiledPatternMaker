#pragma once
#ifndef IRREGULAR_ROSETTE_H
#define IRREGULAR_ROSETTE_H

#include "motifs/irregular_motif.h"

struct Branch
{
    QPointF tipPoint;
    QPointF qePoint;
    QPointF fPoint;
    int     side;
    int     isign;

    void  dump() { qDebug() << "tip" << tipPoint << "qe" << qePoint << "f" << fPoint << "side" << side << "sign" << isign; }
};

class IrregularRosette : public IrregularMotif
{
public:
    IrregularRosette();
    IrregularRosette(const Motif & other);

    void                init(qreal q, qreal r, int s);

    virtual void        buildMotifMaps()  override;
    virtual QString     getMotifDesc()    override { return "IrregularRosette"; }
    virtual void        report()          override { qDebug().noquote() << getMotifDesc() << "q" << q << "r" << r << "s" << s; }

protected:
    void                inferRosette(TilePtr tile);

    void                buildV1(TilePtr tile);
    Branch              buildRosetteBranchPointsV1(int side, int isign);

    void                buildV2(TilePtr tile);
    Branch              buildRosetteBranchPointsV2(int side, int isign, qreal sideLen);

    Points              buildRosetteIntersections(const Branch &branch);
    Branch &            findBranch(int side, int sign);

private:
    Points              corners;        // open
    Points              mids;           // open
    QPointF             center;

    QVector<Branch>     branches;

    int                 debugSide;
};

#endif // IRREGULAR_ROSETTE_H
