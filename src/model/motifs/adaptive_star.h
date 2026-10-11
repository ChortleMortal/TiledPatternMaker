#pragma once

#include "model/motifs/adaptive_star_branches.h"

class AdaptiveStar : public AdaptiveStarBranches
{
public:
    AdaptiveStar();
    AdaptiveStar(const Motif & other);

    void                init(qreal d, int s);

    virtual QString     getMotifDesc()    override { return "AdaptiveStar"; }
    virtual void        dump() override { qDebug().noquote() << getMotifDesc() << "d" << d << "s" << s; }
    virtual int         maxVersions()   override { return 3; }

protected:
    void                infer() override;  // Star inferring

 private:
    void                inferStarv1();
    void                inferStarV2();
    void                inferStarV3();

    QVector<QPointF>    getBranchIsectsV3(int side, int sign);

    QVector<QPointF>    isects;
    QPointF             center;
};
