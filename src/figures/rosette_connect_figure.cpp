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
#include "figures/rosette_connect_figure.h"
#include "geometry/map.h"

RosetteConnectFigure::RosetteConnectFigure(int nn, qreal q, int s, qreal k)
    : Rosette(nn,q,s,k), FigureConnector(this)
{
    setFigureScale(computeConnectScale());
    setFigType(FIG_TYPE_CONNECT_ROSETTE);
}

RosetteConnectFigure::RosetteConnectFigure(const Figure & fig, int nn, qreal q, int s, qreal k)
    : Rosette(fig, nn,q,s,k), FigureConnector(this)
{
    setFigureScale(computeConnectScale());
    setFigType(FIG_TYPE_CONNECT_ROSETTE);
}
qreal RosetteConnectFigure::computeConnectScale()
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    setFigureScale(1.0);
    qreal rot = getFigureRotate();      //save
    setFigureRotate(0.0);
    MapPtr cunit = Rosette::buildUnit();
    setFigureRotate(rot);            // restore
    qreal sc = computeScale(cunit);

    resetMaps();   // so unit can build

    return sc;
}


MapPtr RosetteConnectFigure::buildUnit()
{
    qDebug() << "RosetteConnectFigure::buildUnit";

    // save
    qreal scale = getFigureScale();
    qreal rot   = getFigureRotate();
    setFigureScale(1.0);
    setFigureRotate(0.0);

    // build Rosette
    auto amap = Rosette::buildUnit();
    Q_ASSERT(!amap->isEmpty());
    //unitMap->dump();

    // restore
    setFigureScale(scale);  // erases unit map
    setFigureRotate(rot);   // erases unit map

    Q_ASSERT(!amap->isEmpty());
    unitMap = amap;
    Q_ASSERT(!unitMap->isEmpty());

    // extend Rosette
    connectFigure(unitMap);
    Q_ASSERT(!unitMap->isEmpty());
    //unitMap->dump();

    // We want the tip of the new figure to still be at (1,0).
    // To accomplish this, move the top half of the figure around
    // to lie under the bottom half.  This rebuilds the tip
    // at the correct location.

    rotateHalf(unitMap);  // DAC methinks not needed
    Q_ASSERT(!unitMap->isEmpty());

    unitMap->transformMap(QTransform().rotateRadians(M_PI * don));

    scaleToUnit(unitMap);

    unitMap->verify();

    return unitMap;
}
