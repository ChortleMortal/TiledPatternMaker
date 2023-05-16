#pragma once
#ifndef IRREGULAR_STAR_H
#define IRREGULAR_STAR_H

#include "motifs/irregular_star_branches.h"

class IrregularStar : public IrregularStarBranches
{
public:
    IrregularStar();
    IrregularStar(const Motif & other);

    void                init(qreal d, int s);

    virtual void        buildMotifMaps()  override;
    virtual QString     getMotifDesc()    override { return "IrregularStar"; }
    virtual void        report() override { qDebug().noquote() << getMotifDesc() << "d" << d << "s" << s; }

protected:
    void                inferStar(TilePtr tile);  // Star inferring

 private:
    void                inferStarV1(TilePtr tile);
    void                inferStarV2(TilePtr tile);
    void                inferStarV3(TilePtr tile);

    QVector<QPointF>    getBranchIsectsV3(int side, int sign);

    QVector<QPointF>    isects;
    QPointF             center;
};

#endif // IRREGULAR_STAR_H
