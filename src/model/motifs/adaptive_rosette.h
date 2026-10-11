#pragma once

#include "model/motifs/adaptive_motif.h"

typedef std::shared_ptr<class AdaptiveStar> IStarPtr;

struct Branch
{
    QPointF tipPoint;
    QPointF qePoint;
    QPointF fPoint;
    int     side;
    int     isign;

    void  dump() { qDebug() << "tip" << tipPoint << "qe" << qePoint << "f" << fPoint << "side" << side << "sign" << isign; }
};

class AdaptiveRosette : public AdaptiveMotif
{
public:
    AdaptiveRosette();
    AdaptiveRosette(const Motif & other);

    void                init(qreal q, qreal r, int s, qreal k);

    virtual QString     getMotifDesc()  override { return QString("AdaptiveRosette version %1").arg(getVersion()); };
    virtual void        dump()          override { qDebug().noquote() << getMotifDesc() << "q" << q << "r" << r << "s" << s << "k" << k; }
    virtual int         maxVersions()   override { return 3; }

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
