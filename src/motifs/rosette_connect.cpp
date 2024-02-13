////////////////////////////////////////////////////////////////////////////
//
// ConnectFigure.java
//
// A ConnectFigure is a special kind of ScaleFigure.  It knows how to
// compute just the right scale factor so that scaled out edges will join
// up to create a fancier figure.  This is how we turn Rosettes into
// Extended Rosettes.  To make sure that the resulting figure still lines
// up with the tile that will eventually contain it, we need to do
// some fancy reshuffling of the basic unit to move the apex to (1,0).

#include <QDebug>
#include <QtMath>
#include "motifs/rosette_connect.h"
#include "geometry/map.h"

RosetteConnect::RosetteConnect(int nn, qreal q, int s) : Rosette(nn,q,s)
{
    setMotifType(MOTIF_TYPE_CONNECT_ROSETTE);
}

RosetteConnect::RosetteConnect(const Motif & fig, int nn, qreal q, int s) : Rosette(fig, nn,q,s)
{
    setMotifType(MOTIF_TYPE_CONNECT_ROSETTE);
}

RosetteConnect::RosetteConnect(const RosetteConnect & other) : Rosette(other)
{
    setMotifType(MOTIF_TYPE_CONNECT_ROSETTE);
}

qreal RosetteConnect::computeConnectScale()
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    Rosette::buildUnitMap();
    qreal sc = connector.computeScale(this);
    return sc;
}

void RosetteConnect::buildUnitMap()
{
    qDebug() << "RosetteConnect::buildUnitMap";

    qreal cscale = computeConnectScale();

    connector.connectMotif(this,cscale); // extend Rosette

    // We want the tip of the new figure to still be at (1,0).
    // To accomplish this, move the top half of the figure around
    // to lie under the bottom half.  This rebuilds the tip
    // at the correct location.

    unitMap->transform(QTransform().rotateRadians(M_PI * don));

    connector.scaleToUnit(this);

    setMotifScale(1.0);

    //unitMap->verify();
}
