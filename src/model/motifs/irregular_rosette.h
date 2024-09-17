#pragma once
#ifndef IRREGULAR_ROSETTE_H
#define IRREGULAR_ROSETTE_H

#include "model/motifs/irregular_motif.h"

typedef std::shared_ptr<class IrregularStar> IStarPtr;

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

    virtual QString     getMotifDesc()    override { return "IrregularRosette"; }
    virtual void        dump()          override { qDebug().noquote() << getMotifDesc() << "q" << q << "r" << r << "s" << s; }

protected:
    void                infer() override;

    void                buildv1();
    Branch              buildRosetteBranchPointsV1(int side, int isign);

    void                buildV2();
    Branch              buildRosetteBranchPointsV2(int side, int isign, qreal sideLen);

    void                buildV3();
    Branch              buildRosetteBranchPointsV3(int side, int isign, qreal sideLen);

    Points              buildRosetteIntersections(const Branch &branch);
    Branch &            findBranch(int side, int sign);

private:
    Points              corners;        // open
    Points              mids;           // open
    QPointF             center;

    QVector<Branch>     branches;
    QVector<QPointF>    starPts;

    int                 debugSide;
    bool                _debug;
};

#endif // IRREGULAR_ROSETTE_H
