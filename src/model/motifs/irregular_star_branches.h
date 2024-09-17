#pragma once
#ifndef IRREGULAR_STAR_BRANCHES_H
#define IRREGULAR_STAR_BRANCHES_H

#include "model/motifs/irregular_motif.h"

class IrregularStarBranches : public IrregularMotif
{
public:
    IrregularStarBranches();
    IrregularStarBranches(const Motif & other);
    IrregularStarBranches(const IrregularStarBranches & other);

protected:
    MapPtr      buildStarHalfBranchV1(qreal d, int s, qreal side_frac, int sign);
    MapPtr      buildStarHalfBranchV2 (qreal d, int s, int side, int sign);
    QLineF      getRay(int side, qreal d, int sign);
    static QPointF getArc(qreal frac, const QPolygonF & pts);

    QPolygonF   mids;
    QPolygonF   corners;

private:
    QPolygonF        buildStarBranchPointsV1(qreal d, int s, qreal side_frac, int sign);
    QVector<QPointF> buildStarBranchPointsV2(qreal d, int s, int side, int sign);
};

#endif
