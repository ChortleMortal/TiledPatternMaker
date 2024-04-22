#pragma once
#ifndef STAR2_H
#define STAR2_H

// The classic [n/d]s star construction.  See the paper for moredetails.

#include "motifs/radial_motif.h"

class Star2 : public RadialMotif
{
public:
    // n = points, d = sides hops, s = sides intersections
    Star2(int n, qreal theta, int intersects);
    Star2(const Motif & fig, int n, qreal theta, int intersects);
    Star2(const Star2 & other);

    virtual void buildUnitMap() override;

    void    setS(int ss)            { s = ss; }
    void    setTheta(qreal theta)   { this->theta  = theta; }

    int     getS()                  {return s;}
    qreal   getTheta()              {return theta; }

    bool    equals(const MotifPtr other) override;

    virtual QString getMotifDesc() override { return "Star2"; }
    virtual void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s; }

protected:
    int     clampRays();
    QLineF  getRay(int side, qreal theta, bool sign);

private:
    qreal       theta;
    Points      mids;
    Points      corners;
    QPolygonF   points;
};

#endif

