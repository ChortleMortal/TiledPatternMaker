// The classic [n/d]s star construction.  See the paper for moredetails.

#ifndef STAR_H
#define STAR_H

#include "motifs/radial_motif.h"

class Star : public RadialMotif
{
public:
    // n = points, d = sides hops, s = sides intersections
    Star(int n, qreal dd, int ss);
    Star(const Motif & fig, int n, qreal dd, int ss);
    Star(const Star & other);

    virtual void buildUnitMap() override;

    qreal   getD()  {return d;}
    int     getS()  {return s;}

    void    setD(qreal dd);
    void    setS(int ss);

    virtual QString getMotifDesc() override { return "Star"; }

    bool equals(const MotifPtr other) override;

protected:
    qreal clamp_d(qreal d);
    int   clamp_s(int s);

private:
    qreal 	d;
    int		s;
};

#endif

