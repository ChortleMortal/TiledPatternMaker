#ifndef EXPLICIT_FIGURE_H
#define EXPLICIT_FIGURE_H

#include "figures/figure.h"

////////////////////////////////////////////////////////////////////////////
//
// ExplicitFigure.java
//
// A variety of Figure which contains an explicit map, which is
// simple returned when the figure is asked for its map.

class ExplicitFigure : public Figure
{
public:

    ExplicitFigure(MapPtr map, eFigType figType, int sides);
    ExplicitFigure(const Figure & fig, MapPtr map, eFigType figType, int sides);
    ExplicitFigure(const Figure & fig, eFigType figType, int sides);

    virtual ~ExplicitFigure() override;

    virtual void buildMaps() override;
    virtual void resetMaps() override;

    void   setExplicitMap(MapPtr map) { figureMap = map; }
    MapPtr getFigureMap() override;

    virtual QString getFigureDesc()  override { return "ExplicitFigure"; }

    bool equals(const FigurePtr other) override;

    // a miscellany (hodge-podge)
    qreal   skip;           // girih
    qreal   d;              // hourglass + intersect + star
    int     s;              // hourglass + intersect + star
    qreal   q;              // rosette
    qreal   r_flexPt;       // rosette
    bool    progressive;    // intersect

protected:
    void    init(int sides);
};

#endif

