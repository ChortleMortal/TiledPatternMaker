#pragma once

////////////////////////////////////////////////////////////////////////////
//
// Rosette2
//
// The rosette is a classic feature of Islamic art.  It's a star
// shape surrounded by hexagons.
//
// This class implements the rosette as a RadialFigure using the
// geometric construction given in Lee [1].
//
// [1] A.J. Lee, _Islamic Star Patterns_.  Muqarnas 4.
//
// Rosette2 is an alternative to Kaplan's Rosette, which relaxes
// Kaplans constraint on Knee position which is only a partial truth
//

#include "model/motifs/radial_motif.h"

enum class TipType
{
    REPEAT,
    OUTER,
    INNER,
    FLIPPED,
    OUTER_POINT,
    OUTER_TO_BOUNDARY,          // as OUTER, then the extender moves the tip to its boundary
    OUTER_POINT_TO_BOUNDARY,    // as OUTER_POINT, then the extender moves the tip to its boundary
};

extern  const QString sTipType[];

class Rosette;

class Rosette2 : public RadialMotif
{
public:
    Rosette2(int n, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c);
    Rosette2(const Rosette2 & other);
    Rosette2(const Motif & motif);
    Rosette2(const Rosette & rosette);

    virtual void  buildUnitRays()   override;

    qreal   getKneeX()              { return kneeX;}
    qreal   getKneeY()              { return kneeY;}
    qreal   getK()                  { return k;}
    int     getS()                  { return s;}
    bool    getConstrain()          { return constrain; }

    void    setKneeX(qreal x)       { kneeX = x; }
    void    setKneeY(qreal y)       { kneeY = y; }
    void    setK(qreal kk)          { k = kk; }
    void    setS(int ss)            { s = ss; }
    void    setConstrain(bool c)    { constrain = c; }

    void             initTipTypes()                         { tipTypes.push_back(TipType::OUTER); }
    void             setTipTypes(QList<TipType> tiptypes)   { tipTypes = tiptypes; }
    QList<TipType> & getTipTypes()                          { return tipTypes; }
    TipType          getTipType(int tip);
    static bool      isBoundaryTip(TipType tt)              { return tt == TipType::OUTER_TO_BOUNDARY || tt == TipType::OUTER_POINT_TO_BOUNDARY; }

    bool    hasTipsToMove()                     override;
    bool    moveTipToBoundary(int rayIndex)     override;

    virtual QString getMotifDesc() override { return "Rosette2";}
    virtual void    dump()         override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "kneeX:" << kneeX << "kneeY:" << kneeY << "s:" << s << "k:" << k
                                            << "extenders" << getExtenders().count() ; }
    virtual int     maxVersions()  override { return 1; }

    bool    equals(const MotifPtr other) override;
    bool    pointOnLineLessThan(QPointF p1, QPointF v2);
    bool    convertConstrained();

protected:
    int     buildUnitRays(QPointF tip, const RadialRay &epoints, int side);
    void    calcConstraintLine();

private:
    QList<TipType> tipTypes;
    QList<TipType> unitTipTypes;    // generated - tip type of each LR pair in unitRays

    bool     constrain;     // use Kaplan's constraint
    QLineF   constraint;    // Kaplan's aesthetic call of line where knee should lie on

    QPointF  kneePt;        // used by sort

    qreal    yRange;
    qreal    xRange;
};

