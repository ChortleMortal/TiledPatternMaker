#pragma once

// The classic [n/d]s star construction.  See the paper for moredetails.

#include "model/motifs/radial_motif.h"
#include "model/tilings/tile.h"

class Star : public RadialMotif
{
    friend class MotifView;

public:
    // n = points, d = sides hops, s = sides intersections
    Star(int n, qreal dd, int ss);
    Star(const Star & other);
    Star(const Motif & other);

    virtual void buildUnitRays() override;

    void    setS(int ss);
    void    setD(qreal dd);

    int     getS()  {return s;}
    qreal   getD()  {return d;}

    bool    equals(const MotifPtr other) override;

    virtual QString getMotifDesc() override { return QString("Star version %1").arg(getVersion()); }
    virtual void    dump()  override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s
                                << "extenders" << getExtenders().count() ; }
    virtual int     maxVersions()   override { return 2; }

protected:
    void    buildUnitV1();
    void    buildUnitV2();

    qreal   clamp_d(qreal d);
    int     clamp_s(int s);

    QLineF  getRay(int side, qreal d, int sign);
    void    createTileAndMids();

private:
    Points      mids;
    TilePtr     tile;
};

