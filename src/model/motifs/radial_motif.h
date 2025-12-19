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

#include "model/motifs/motif.h"
#include "model/motifs/radial_ray.h"

class RadialMotif : public Motif
{
public:
    virtual ~RadialMotif() {}

    virtual void    resetMotifMap() override;
            void    buildMotifMap() override final;

    virtual MapPtr  getMotifMap()   override;  // Get a complete map from unit.

    QTransform      getMotifTransform() override;
    QTransform      getDELTransform()   override;

    virtual void    buildUnitMap() = 0;

    virtual void    setN(int n)     override;

    virtual MapPtr  getUnitMap()  const         { return unitMap; }
    virtual MapPtr  getUnitMap2() const         { return unitMap2; }
    qreal           get_don()                   { return don; }
    QTransform      getUnitRotationTransform()  {return radialRotationTr;}

    RaySet &        getRaySet1()                { return raySet1; }
    RaySet &        getRaySet2()                { return raySet2; }

    static QPointF  getArc( qreal frac );       // Get the point frac of the way around the unit circle.

    qreal           computeConnectScale();

    void            setOnPoint(bool enb);
    void            setInscribe(bool enb);
    bool            getOnPoint()                { return onPoint; }
    bool            getInscribe()               { return inscribe; }

    virtual QString getMotifDesc()   override   { return "RadialMotif"; }
    virtual void    dump()           override   {};

protected:
    RadialMotif(int n);
    RadialMotif(const Motif & motif, int n);
    RadialMotif(const RadialMotif & motif);

    virtual MapPtr  createUnitMapFromRays(RaySet & set);

    void            setupRadialTransform();     // Transform for eachradial popint/branch
    virtual void    replicate();

    // data
    qreal   d;      // used by star
    int     s;      // used by star + rosette
    qreal   q;      // used by rosette
    qreal   kneeX;  // used by rosette2
    qreal   kneeY;  // used by rosette2
    qreal   k;      // used by rosette2
    bool    inscribe;
    bool    onPoint;

    // generated
    qreal       don;
    QTransform  radialRotationTr;
    MapPtr      unitMap;
    MapPtr      unitMap2;
    RaySet      raySet1;
    RaySet      raySet2;
};

typedef std::shared_ptr<RadialMotif>    RadialMotifPtr;

#endif

