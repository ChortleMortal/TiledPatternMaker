#pragma once

#include "model/motifs/adaptive_motif.h"
#include "model/motifs/adaptive_tools.h"

class AdaptiveGirihBranches : public AdaptiveMotif
{
public:
    AdaptiveGirihBranches();
    AdaptiveGirihBranches(const Motif & other);
    AdaptiveGirihBranches(const AdaptiveGirihBranches & other);

protected:
    MapPtr           buildGirihBranch(int side, qreal requiredRotation);
    QPointF          buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation);
    IntersectionInfo findClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation);

    QPolygonF   mids;
    QPolygonF   corners;
private:
};
