#pragma once
#ifndef INTERSECT_MOTIF_H
#define INTERSECT_MOTIF_H

#include "motifs/irregular_girih_branches.h"

class EdgesLengthInfo
{
public:
    EdgesLengthInfo(int side1, bool isLeft1, int side2, bool isLeft2, int ic, qreal dist2, QPointF i);

    bool    equals  (  EdgesLengthInfo & other );
    int     compareTo( EdgesLengthInfo & other );

    void    dump();

    static bool lessThan(const EdgesLengthInfo *&e1, const EdgesLengthInfo *&e2);

    int     side1;          // Which side of the tile this describes.
    bool    isLeft1;        // True if first side is left edge.
    int     side2;          // Which side of the tile this describes.
    bool    isLeft2;        // True if second side is left edge.
    int     intersection_count;
    qreal   dist2;          // The square of the distance (square to avoid a square root op.)
    QPointF intersection;   // The intersection point.
};


class IntersectMotif : public IrregularGirihBranches
{
public:
    IntersectMotif();
    IntersectMotif(const Motif & other);

    void init(qreal starSkip, int s, bool progressive);

    virtual void    buildMotifMaps()  override;
    virtual QString getMotifDesc()    override { return "IntersectMotif"; }
    virtual void    report() override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "skip:" << skip << "s:" << s << "progressive:" << progressive; }

    QList<EdgesLenPtr> buildIntersectEdgesLengthInfos(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation);
    QList<IntersectionPtr> buildIntersectionInfos    (int side,QPointF sideHalf, bool isLeftHalf, qreal requiredRotation);
    int getIntersectionRank(int side, bool isLeft, QList<IntersectionPtr> infos);

protected:
    void inferIntersect(TilePtr tile);              // Intersect inferring
    void inferIntersectProgressive(TilePtr tile);  // Progressive intersect inferring
};

#endif // INTERSECT_MOTIF_H
