#pragma once

#include "model/motifs/adaptive_motif.h"

class AdaptiveStarBranches : public AdaptiveMotif
{
public:
    AdaptiveStarBranches();
    AdaptiveStarBranches(const Motif & other);
    AdaptiveStarBranches(const AdaptiveStarBranches & other);

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
