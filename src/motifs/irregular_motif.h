#pragma once
#ifndef IRREGULAR_MOTIF_H
#define IRREGULAR_MOTIF_H

#include "motifs/motif.h"

class IrregularMotif;

typedef std::shared_ptr<class Prototype>  ProtoPtr;

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMotif.java
//
// A variety of motif which contains an explicit map, which is
// simple returned when the motif is asked for its map.

class IrregularMotif : public Motif
{
public:
    IrregularMotif();
    IrregularMotif(const Motif & other);
    IrregularMotif(MotifPtr other);

    bool equals(const MotifPtr other) override;

    virtual void    resetMotifMaps() override;
    MapPtr          getMotifMap()    override;
    virtual void    buildMotifMaps() override;
    virtual QString getMotifDesc()   override  { return "IrregularMotif"; }
    virtual void    setup(TilePtr tile)        { this->tile = tile; }

    void dump();
    virtual void    report()         override { qDebug().noquote() << getMotifDesc(); }

    // data a miscellany (hodge-podge)
    qreal   skip;           // girih
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette
    qreal   r;              // rosette flex point
    bool    progressive;    // intersect

protected:


    MapPtr          completeMotif(MapPtr map);
    MapPtr          completeMap(MapPtr map);

    TilePtr         tile;

private:
    void            init();
};

#endif
