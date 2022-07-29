////////////////////////////////////////////////////////////////////////////
//
// RadialFigure.java
//
// A RadialFigure is a special kind of Figure that has d_n symmetry.  That
// means that it can be rotated by 360/n degrees and flipped across certain
// lines through the origin and it looks the same.
//
// We take advantage of this by only making subclasses produce a basic
// unit, i.e. a smaller map that generates the complete figure through the
// action of c_n (just the rotations; the reflections are factored in
// by subclasses).

#ifndef RADIAL_FIGURE_H
#define RADIAL_FIGURE_H

#include "figures/figure.h"

class RadialFigure : public Figure
{
public:
    virtual ~RadialFigure() override {}

    virtual void buildMaps() override;
    virtual void resetMaps() override;

    // Get a complete map from unit.
    virtual MapPtr  getFigureMap() override;

    virtual void    setN(int n) override;

    qreal   get_dn()  { return dn; }
    qreal   get_don() { return don; }
    QTransform getTransform() {return Tr;}

    virtual MapPtr   buildUnit() = 0;
    virtual MapPtr   useBuiltMap() const { return figureMap; }
    virtual MapPtr   useUnitMap() const { return unitMap; }
    virtual QString  getFigureDesc() override { return "Radial Figure"; }

    MapPtr           replicateUnit();

    // Get the point frac of the way around the unit circle.
    static QPointF getArc( qreal frac );

protected:
    RadialFigure(int n);
    RadialFigure(const Figure & fig, int n);

    virtual void     buildExtBoundary() override;

    qreal         dn;
    qreal         don;
    QTransform    Tr;

    MapPtr        unitMap;
};

#endif

