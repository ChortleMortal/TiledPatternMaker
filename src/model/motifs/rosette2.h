#pragma once
#ifndef ROSETTE2_H
#define ROSETTE2_H

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

enum eTipTypes
{
    TIP_TYPE2_OUTER     = 0x01,
    TIP_TYPE2_INNER     = 0x02,
    TIP_TYPE2_FLIPPED   = 0x04,
};

enum eTipMode
{
    TIP_MODE_REGULAR,
    TIP_MODE_ALTERNATE,
};

class Rosette2 : public RadialMotif
{
public:
    Rosette2(const Motif & fig,  int n, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c);
    Rosette2(int n, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c);
    Rosette2(const Rosette2 & other);

    virtual void  buildUnitMap()   override;

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

    void    addTipType(eTipTypes t)     { tipTypes |= t; }
    void    removeTipType(eTipTypes t)  { tipTypes &= ~t; }
    void    setTipTypes(uint tt)        { tipTypes = tt; }
    uint    getTipTypes()               { return tipTypes; }

    void    setTipMode(eTipMode mode)   { _tipMode = mode; }
    eTipMode getTipMode() const         { return _tipMode; }

    virtual QString getMotifDesc() override { return "Rosette2";}
    virtual void    dump()         override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "kneeX:" << kneeX << "kneeY:" << kneeY << "s:" << s << "k:" << k
                                            << ((getRadialConnector()) ? "has connector" : "no connector")
                                            << "extenders" << getExtenders().count() ; }

    bool    equals(const MotifPtr other) override;
    bool    pointOnLineLessThan(QPointF p1, QPointF v2);
    bool    convertConstrained();

protected:
    void    buildRay(RaySet &set, QPointF tip, const QVector<QPointF> & epoints);
    void    calcConstraintLine();

private:
    eTipMode _tipMode;
    uint     tipTypes;
    bool     constrain;     // use Kaplan's constraint
    QLineF   constraint;    // Kaplan's aesthetic call of line where knee should lie on

    QPointF  kneePt;        // used by sort

    qreal    yRange;
    qreal    xRange;
};

#endif

