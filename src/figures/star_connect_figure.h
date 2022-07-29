////////////////////////////////////////////////////////////////////////////
//
// ConnectFigure.java
//
// A ConnectFigure is a special kind of ScaleFigure.  It knows how to
// compute just the right scale factor so that scaled out edges will join
// up to create a fancier figure.  This is how we turn Rosettes into
// Extended Rosettes.  To make sure that the resulting figure still lines
// up with the feature that will eventually contain it, we need to do
// some fancy reshuffling of the basic unit to move the apex to (1,0).

#ifndef STAR_CONNECTFIGURE_H
#define STAR_CONNECTFIGURE_H

#include "figures/star.h"
#include "figures/figure_connector.h"

class StarConnectFigure : public Star, FigureConnector
{
public:
    StarConnectFigure(int n, qreal d, int s);
    StarConnectFigure(const Figure & fig, int n, qreal d, int s);

    MapPtr buildUnit() override;

    virtual QString getFigureDesc() override { return "Star Connect figure";}

    qreal computeConnectScale();
};

#endif

