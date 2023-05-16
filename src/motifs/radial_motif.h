#pragma once
#ifndef RADIAL_MOTIF_H
#define RADIAL_MOTIF_H

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

#include "motifs/motif.h"

class RadialMotif : public Motif
{
public:
    virtual ~RadialMotif() override {}

    virtual MapPtr  getMotifMap() override; // Get a complete map from unit.
    virtual void    resetMotifMaps() override;
    virtual void    buildMotifMaps() override;

    virtual void    buildUnitMap() = 0;
    virtual MapPtr  getUnitMap() const { return unitMap; }

    void            buildRadialBoundaries();

    virtual void    setN(int n) override;

    qreal           get_don()       { return don; }
    QTransform      getTransform()  {return Tr;}

    static QPointF  getArc( qreal frac );    // Get the point frac of the way around the unit circle.

    virtual QString getMotifDesc() override { return "RadialMotif"; }
    virtual void    report() override = 0;

protected:
    RadialMotif(int n);
    RadialMotif(const Motif & motif, int n);
    RadialMotif(const RadialMotif & motif);

    void        setupRadialTransform();      // Transform for eachradial popint/branch
    void        replicate();

    // data
    qreal       d;  // used by star
    int         s;  // used by star + rosette
    qreal       q;  // used by rosette
    qreal       k;  // used by rosette

    // generated
    qreal       don;
    QTransform  Tr;
    MapPtr      unitMap;
};

#endif

