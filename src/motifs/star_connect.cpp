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

#include "motifs/star_connect.h"
#include "geometry/map.h"

StarConnect::StarConnect(int nsides, qreal d, int s) : Star(nsides,d,s)
{
    setMotifType(MOTIF_TYPE_CONNECT_STAR);
}

StarConnect::StarConnect(const Motif & motif, int nsides, qreal d, int s) : Star(motif, nsides,d,s)
{
    setMotifType(MOTIF_TYPE_CONNECT_STAR);
}

StarConnect::StarConnect(const StarConnect & other) : Star(other)
{
}

qreal StarConnect::computeConnectScale()
{
    // Find the vertex at (1,0), extend its incoming edges, intersect
    // with rotations of same, and use the location of the intersection
    // to compute a scale factor.

    Star::buildUnitMap();
    qreal sc = connector.computeScale(this);
    return sc;
}

void StarConnect::buildUnitMap()
{
    qDebug() << "StarConnect::buildUnit";

    qreal cscale = computeConnectScale();

    connector.connectMotif(this,cscale);

    // We want the tip of the new figure to still be at (1,0).
    // To accomplish this, move the top half of the figure around
    // to lie under the bottom half.  This rebuilds the tip
    // at the correct location.

    unitMap->transform(QTransform().rotateRadians(M_PI * don));

    connector.scaleToUnit(this);

    setMotifScale(1.0);

    unitMap->verify();
}
