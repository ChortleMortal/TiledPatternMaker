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

#include <QDebug>
#include <QtMath>

#include "figures/star_connect_figure.h"
#include "geometry/map.h"

StarConnectFigure::StarConnectFigure(int nsides, qreal d, int s)
    : Star(nsides,d,s), FigureConnector(this)
{
    setFigureScale(computeConnectScale());
    setFigType(FIG_TYPE_CONNECT_STAR);
}

StarConnectFigure::StarConnectFigure(const Figure & fig, int nsides, qreal d, int s)
    : Star(fig, nsides,d,s), FigureConnector(this)
{
    setFigureScale(computeConnectScale());
    setFigType(FIG_TYPE_CONNECT_STAR);
}

qreal StarConnectFigure::computeConnectScale()
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    setFigureScale(1.0);
    qreal rot = getFigureRotate();      //save
    setFigureRotate(0.0);
    MapPtr cunit = Star::buildUnit();
    setFigureRotate(rot);            // restore
    qreal sc = computeScale(cunit);

    resetMaps();        // so unit can build

    return sc;
}

MapPtr StarConnectFigure::buildUnit()
{
    qDebug() << "StarConnectFigure::buildUnit";

    // save
    qreal scale = getFigureScale();
    qreal rot   = getFigureRotate();
    setFigureScale(1.0);
    setFigureRotate(0.0);

    // build Rosette
    unitMap = Star::buildUnit();
    //unitMap->dump();

    // restore
    setFigureScale(scale);
    setFigureRotate(rot);

    // extend Rosette
    connectFigure(unitMap);
    //unitMap->dump();

    // We want the tip of the new figure to still be at (1,0).
    // To accomplish this, move the top half of the figure around
    // to lie under the bottom half.  This rebuilds the tip
    // at the correct location.

    rotateHalf(unitMap);  // DAC methinks not needed

    QTransform t = QTransform().rotateRadians(M_PI * don);
    unitMap->transformMap(t);

    scaleToUnit(unitMap);

    unitMap->verify();

    return unitMap;
}
