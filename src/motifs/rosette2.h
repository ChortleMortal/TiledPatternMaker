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

#include "motifs/radial_motif.h"

enum eTipType
{
    TIP_TYPE_OUTER,
    TIP_TYPE_INNER,
    TIP_TYPE_ALTERNATE
};

class Rosette2 : public RadialMotif
{
public:
    Rosette2(const Motif & fig,  int n, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c);
    Rosette2(int n, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c);
    Rosette2(const Rosette2 & other);

    virtual void  buildUnitMap() override;

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

    void     setTipType(eTipType t) { tipType = t; }
    eTipType getTipType()           { return tipType; }

    virtual QString getMotifDesc() override { return "Rosette2";}
    virtual void    report()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "kneeX:" << kneeX << "kneeY:" << kneeY << "s:" << s << "k:" << k; }

    bool    equals(const MotifPtr other) override;
    bool    pointOnLineLessThan(QPointF p1, QPointF v2);
    bool    convertConstrained();

protected:
    void    replicate() override;

    void    buildSegement(MapPtr map, QPointF tip, const QVector<QPointF> &epoints);
    void    calcConstraintLine();

private:
    eTipType tipType;
    bool     constrain;     // use Kaplan's constraint
    QLineF   constraint;    // Kaplan's aesthetic call of line where knee should lie on

    QPointF  kneePt;        // used by sort

    MapPtr   unitMap2;
    qreal    yRange;
    qreal    xRange;
};

#endif

