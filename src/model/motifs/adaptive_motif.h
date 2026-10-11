#pragma once

#include "model/motifs/motif.h"

class AdaptiveMotif;

typedef std::shared_ptr<class Prototype>  ProtoPtr;

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMotif.java
//
// A variety of motif which contains an explicit map, which is
// simple returned when the motif is asked for its map.

class AdaptiveMotif : public Motif
{
public:
    AdaptiveMotif();
    AdaptiveMotif(const AdaptiveMotif & other);
    AdaptiveMotif(const Motif & other);
    virtual ~AdaptiveMotif() {}

            void    build()  override final;

    ExtenderPtr     createExtender() override final;

    virtual void    infer() = 0;

    virtual void    irr_extendMaps();
    void            irr_scaleAndRotate();
    void            irr_buildMotifBoundary();

    virtual QString getMotifDesc()      override  { return "AdaptiveMotif"; }
    QTransform      getMotifTransform() override;

    bool equals(const MotifPtr other)   override;

    virtual void    dump()              override;

    // data a miscellany (hodge-podge)
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette

    qreal   skip;           // girih
    qreal   r;              // rosette flex point
    qreal   k;              // rosette knee angle (v2)
    bool    progressive;    // intersect

    bool    cropToTile;     // EXPERIMENTAL - crop the built motif map to the tile boundary

protected:
    void    irr_completeMap();

private:
    void    init();
};

class IrregularNoMap : public AdaptiveMotif
{
public:
    IrregularNoMap();
    IrregularNoMap(const Motif & other);

    void infer() override {};

    virtual int     maxVersions()   override { return 1; }
};
