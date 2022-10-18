////////////////////////////////////////////////////////////////////////////
//
// RadialFigure.java
//
// A RadialMotif is a special kind of Motif that has d_n symmetry.  That
// means that can be rotated by 360/n degrees and flipped across certain
// lines through the origin and it looks the same.
//
// We take advantage of this by only making subclasses produce a basic
// unit, i.e. a smaller map that generates the complete figure through the
// action of c_n (just the rotations; the reflections are factored in
// by subclasses).

#ifndef RADIAL_MOTIF_H
#define RADIAL_MOTIF_H

#include "motifs/motif.h"

class RadialMotif : public Motif
{
public:
    virtual ~RadialMotif() override {}

    virtual MapPtr  getMap() override; // Get a complete map from unit.
    virtual void    resetMaps() override;
    virtual void    buildMaps() override;

    virtual void    buildUnitMap() = 0;
    virtual MapPtr  getUnitMap() const { return unitMap; }

    virtual QString getMotifDesc() override { return "Radial Motif"; }

    void buildRadialBoundaries();

    virtual void    setN(int n) override;

    qreal   get_dn()  { return dn; }
    qreal   get_don() { return don; }
    QTransform getTransform() {return Tr;}


    // Get the point frac of the way around the unit circle.
    static QPointF getArc( qreal frac );

protected:
    RadialMotif(int n);
    RadialMotif(const Motif & motif, int n);
    RadialMotif(const RadialMotif & motif);

    void         replicate();

    qreal         dn;
    qreal         don;
    QTransform    Tr;

    MapPtr        unitMap;
};

#endif

