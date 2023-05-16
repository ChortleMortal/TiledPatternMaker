#pragma once
#ifndef ROSETTE_H
#define ROSETTE_H

////////////////////////////////////////////////////////////////////////////
//
// Rosette.java
//
// The rosette is a classic feature of Islamic art.  It's a star
// shape surrounded by hexagons.
//
// This class implements the rosette as a RadialFigure using the
// geometric construction given in Lee [1].
//
// [1] A.J. Lee, _Islamic Star Patterns_.  Muqarnas 4.

#include "motifs/radial_motif.h"

class Rosette : public RadialMotif
{
public:
    // n = points, q = tip angle, s=sides intersections k=neck r=rotation sc=scale
    Rosette(const Motif & fig,  int n, qreal qq, int ss, qreal kk);
    Rosette(int n, qreal qq, int ss, qreal kk);
    Rosette(const Rosette & other);

    virtual void  buildUnitMap() override;

    qreal   getQ() {return q;}
    qreal   getK() {return k;}
    int     getS() {return s;}

    void    setQ(qreal qq);
    void    setK(qreal kk);
    void    setS(int ss);
    void    setN(int n) override;

    virtual QString getMotifDesc() override { return "Rosette";}
    virtual void    report()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "q:" << q << "s" << s << "k" << k; }

    bool equals(const MotifPtr other) override;

protected:
    inline int     s_clamp(int s);
    inline qreal   q_clamp(qreal q);

};

#endif

