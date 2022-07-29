// The classic [n/d]s star construction.  See the paper for moredetails.

#ifndef STAR_H
#define STAR_H

#include "figures/radial_figure.h"

class Star : public RadialFigure
{
public:
    // n = points, d = sides hops, s = sides intersections
    Star(int n, qreal dd, int ss);
    Star(const Figure & fig, int n, qreal dd, int ss);

    virtual MapPtr buildUnit() override;

    qreal   getD()  {return d;}
    int     getS()  {return s;}

    void    setD(qreal dd);
    void    setS(int ss);

    virtual QString getFigureDesc() override { return "Star"; }

    bool equals(const FigurePtr other) override;

protected:
    qreal clamp_d(qreal d);
    int   clamp_s(int s);

private:
    qreal 	d;
    int		s;
};

#endif

