#pragma once
#ifndef IRREGULAR_STAR_H
#define IRREGULAR_STAR_H

#include "model/motifs/irregular_star_branches.h"

class IrregularStar : public IrregularStarBranches
{
public:
    IrregularStar();
    IrregularStar(const Motif & other);

    void                init(qreal d, int s);

    virtual QString     getMotifDesc()    override { return "IrregularStar"; }
    virtual void        dump() override { qDebug().noquote() << getMotifDesc() << "d" << d << "s" << s; }

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

#endif // IRREGULAR_STAR_H
