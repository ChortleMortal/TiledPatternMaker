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
    virtual ~IrregularMotif() {}

    virtual void    resetMotifMaps()  override;
    virtual void    buildMotifMaps()  override;

    MapPtr          getMotifMap()     override;
    virtual QString getMotifDesc()    override  { return "IrregularMotif"; }
    QTransform      getMotifTransform() override;

    bool equals(const MotifPtr other) override;

    virtual void    dump()          override;

    // data a miscellany (hodge-podge)
    qreal   skip;           // girih
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette
    qreal   r;              // rosette flex point
    bool    progressive;    // intersect

protected:
    void    completeMap();

private:
    void    init();
};

#endif
