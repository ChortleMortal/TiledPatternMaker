#pragma once
#ifndef IRREGULAR_GIRIH_BRANCHES_H
#define IRREGULAR_GIRIH_BRANCHES_H

#include "model/motifs/irregular_motif.h"
#include "model/motifs/irregular_tools.h"

class IrregularGirihBranches : public IrregularMotif
{
public:
    IrregularGirihBranches();
    IrregularGirihBranches(const Motif & other);
    IrregularGirihBranches(const IrregularGirihBranches & other);

protected:
    MapPtr           buildGirihBranch(int side, qreal requiredRotation);
    QPointF          buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation);
    IntersectionInfo findClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation);

    QPolygonF   mids;
    QPolygonF   corners;
private:
};

#endif
