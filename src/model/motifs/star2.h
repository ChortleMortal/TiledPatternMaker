#pragma once

// The classic [n/d]s star construction.  See the paper for moredetails.

#include "model/motifs/radial_motif.h"


class Star2 : public RadialMotif
{
public:
    // n = points, d = sides hops, s = sides intersections
    Star2(int n, qreal theta, int intersects);
    Star2(const Star2 & other);
    Star2(const Motif & motif);

    virtual void buildUnitRays()  override;

    void    setS(int ss)            { s = ss; }
    void    setTheta(qreal theta)   { this->theta  = theta; }

    int     getS()                  {return s;}
    qreal   getTheta()              {return theta; }

    bool    equals(const MotifPtr other) override;

    virtual QString getMotifDesc() override { return "Star2"; }
    virtual void    dump()  override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s
                               << "extenders" << getExtenders().count() ; }
    virtual int     maxVersions()   override { return 1; }
protected:
    QLineF  getRay(int side, qreal theta, bool sign);

private:
    qreal       theta;
    Points      mids;
    Points      corners;
};

