#pragma once

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

#include "model/motifs/radial_motif.h"

class Rosette : public RadialMotif
{
public:
    // n = points, q = tip angle, s=sides intersections
    Rosette(int n, qreal qq, int ss);
    Rosette(const Rosette & other);
    Rosette(const Motif & other);

    virtual void  buildUnitRays() override;

    QLineF  getKeyLine() const;

    qreal   getQ() {return q;}
    int     getS() {return s;}

    void    setQ(qreal qq);
    void    setS(int ss);
    void    setN(int n) override;

    virtual QString getMotifDesc() override { return "Rosette";}
    virtual void    dump()         override { qDebug().noquote()  << getMotifDesc() << "sides:" << getN() << "q:" << q << "s" << s
                                   << "extenders" << getExtenders().count() ; }
    virtual int     maxVersions()  override { return 1; }

    bool    equals(const MotifPtr other) override;

protected:
    inline int     s_clamp(int s);
    inline qreal   q_clamp(qreal q);

};

