#ifndef EXPLICIT_MOTIF_H
#define EXPLICIT_MOTIF_H

#include "motifs/motif.h"
#include "motifs/inference_engine.h"

////////////////////////////////////////////////////////////////////////////
//
// ExplicitMotif.java
//
// A variety of motif which contains an explicit map, which is
// simple returned when the motif is asked for its map.

class ExplicitMotif : public InferenceEngine
{
public:

    ExplicitMotif(MapPtr map, eMotifType motifType, int sides);
    ExplicitMotif(const Motif & motif, MapPtr map, eMotifType motifType, int sides);
    ExplicitMotif(const Motif & motif, eMotifType motifType, int sides);
    ExplicitMotif(const ExplicitMotif & other);

    virtual ~ExplicitMotif() override;

    virtual void    buildMaps() override;
    virtual void    resetMaps() override;
    MapPtr          getMap() override;

    void            setExplicitMap(MapPtr map);

    virtual QString getMotifDesc()  override { return "Explicit Motif"; }

    bool equals(const MotifPtr other) override;

    // a miscellany (hodge-podge)
    qreal   skip;           // girih
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette
    qreal   r_flexPt;       // rosette
    bool    progressive;    // intersect

    void dump();

protected:
    void    init(int sides);

private:
};

#endif

