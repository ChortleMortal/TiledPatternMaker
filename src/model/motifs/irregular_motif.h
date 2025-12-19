#pragma once
#ifndef IRREGULAR_MOTIF_H
#define IRREGULAR_MOTIF_H

#include "model/motifs/motif.h"

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
    IrregularMotif(const IrregularMotif & other);
    IrregularMotif(const Motif & other);
    IrregularMotif(MotifPtr other);
    virtual ~IrregularMotif() {}

    virtual void    resetMotifMap()  override;
            void    buildMotifMap()  override final;
    virtual void    infer() = 0;

    virtual void    irr_extendMaps();
    void            irr_scaleAndRotate(MapPtr map);
    void            irr_buildMotifBoundary();

    MapPtr          getMotifMap()     override;
    virtual QString getMotifDesc()    override  { return "IrregularMotif"; }
    QTransform      getMotifTransform() override;
    QTransform      getDELTransform() override;

    bool equals(const MotifPtr other) override;

    virtual void    dump()            override;

    // data a miscellany (hodge-podge)
    qreal   skip;           // girih
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette
    qreal   r;              // rosette flex point
    bool    progressive;    // intersect

protected:
    void    irr_completeMap();

private:
    void    init();
};

class IrregularNoMap : public IrregularMotif
{
public:
    IrregularNoMap();
    IrregularNoMap(const Motif & other);

    void infer() override {};
};

#endif
