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

#ifndef ROSETTE_H
#define ROSETTE_H

#include "figures/radial_figure.h"

class Rosette : public RadialFigure
{
public:
    // n = points, q = tip angle, s=sides intersections k=neck r=rotation sc=scale
    Rosette(const Figure & fig,  int n, qreal qq, int ss, qreal kk);
    Rosette(int n, qreal qq, int ss, qreal kk);

    virtual MapPtr   buildUnit() override;

    qreal   getQ() {return q;}
    qreal   getK() {return k;}
    int     getS() {return s;}

    void    setQ(qreal qq);
    void    setK(qreal kk);
    void    setS(int ss);
    void    setN(int n) override;

    virtual QString getFigureDesc() override { return "Rosette";}

    bool equals(const FigurePtr other) override;

protected:
    inline int     s_clamp(int s);
    inline qreal   q_clamp(qreal q);

private:
    int     count;
    qreal   q;
    qreal   k;
    int     s;
};

#endif

