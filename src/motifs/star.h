#pragma once
#ifndef STAR_H
#define STAR_H

// The classic [n/d]s star construction.  See the paper for moredetails.

#include "motifs/radial_motif.h"
#include "tile/tile.h"

class Star : public RadialMotif
{
public:
    // n = points, d = sides hops, s = sides intersections
    Star(int n, qreal dd, int ss);
    Star(const Motif & fig, int n, qreal dd, int ss);
    Star(const Star & other);

    virtual void buildUnitMap() override;

    void    setS(int ss);
    void    setD(qreal dd);
    virtual void setN(int n) override;

    int     getS()  {return s;}
    qreal   getD()  {return d;}

    bool    equals(const MotifPtr other) override;

    virtual QString getMotifDesc() override { return "Star"; }
    virtual void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s; }

protected:
    void    buildv1();
    void    buildV2();

    qreal   clamp_d(qreal d);
    int     clamp_s(int s);

    QLineF  getRay(int side, qreal d, int sign);

private:
    Points      mids;
    TilePtr     tile;
    QPolygonF   points;
};

#endif

