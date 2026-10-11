#pragma once

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

    QTransform      getMotifTransform() override;

    void            build()          override final;
    ExtenderPtr     createExtender() override final;

    RaySet &        getUnitRays()               { return unitRays; }
    RaySet &        getFullRays()               { return fullRays; }

    void            setOnPoint(bool enb);
    void            setInscribe(bool enb);
    bool            getOnPoint()                { return onPoint; }
    bool            getInscribe()               { return inscribe; }

    inline qreal    don() const                 { return _don; }
    qreal           getMotifFitScale();

    // the extender moves (rather than adds) tips to its boundary for these rays
    virtual bool    hasTipsToMove()                 { return false; }
    virtual bool    moveTipToBoundary(int rayIndex) { Q_UNUSED(rayIndex); return false; }

protected:
    RadialMotif(int n);
    RadialMotif(const RadialMotif & radial);
    RadialMotif(const Motif & motif);

    virtual void    buildUnitRays() = 0;
    virtual void    buildFullRays();        // replicates
    virtual void    extend();
    virtual void    embellish();
    virtual void    buildMap();

    virtual void    setN(int n)     override;

    void            set_don(qreal val)          { _don = val; }

    QTransform      getRadialTransform(int side);

    static QPointF  getArc( qreal frac );       // Get the point frac of the way around the unit circle.

    virtual void    replicateRays();
    void            showRays(RaySet & rays);

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
    qreal       _don;
    qreal       _unitAngle;
    RaySet      unitRays;
    RaySet      fullRays;
    ExtenderPtr extender;
};

typedef std::shared_ptr<RadialMotif>    RadialMotifPtr;

