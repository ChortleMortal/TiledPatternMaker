////////////////////////////////////////////////////////////////////////////
//
// Star.java
//
// The classic [n/d]s star construction.  See the paper for more details.

#include <QtMath>
#include <QDebug>
#include "motifs/star2.h"
#include "geometry/map.h"
#include "geometry/vertex.h"
#include "tile/tile.h"

typedef std::shared_ptr<Star2> Star2Ptr;

using std::make_shared;

Star2::Star2(int nsides, qreal theta, int intersects) : RadialMotif(nsides)
{
    this->theta = theta;
    s = intersects;
    setMotifType(MOTIF_TYPE_STAR2);
}

Star2::Star2(const Motif & fig,  int nsides, qreal theta, int intersects) : RadialMotif(fig, nsides)
{
    this->theta = theta;
    s = intersects;
    setMotifType(MOTIF_TYPE_STAR2);
}

Star2::Star2(const Star2 & other) : RadialMotif(other)
{
    theta = other.theta;
    s     = other.s;
}

bool Star2::equals(const MotifPtr other)
{
    Star2Ptr otherp = std::dynamic_pointer_cast<Star2>(other);
    if (!otherp)
        return  false;

    if (theta != otherp->theta)
        return  false;

    if (s != otherp->s)
        return false;

    if (!Motif::equals(other))
        return  false;

    return true;
}

void Star2::buildUnitMap()
{
    // int s        = number of intersects
    // qreal theta  = angle

    qDebug() << "Star2::buildUnitMap" << "n:" << getN() << "theta:" << theta << "intersects:" << s;

    Tile tile(getN());
    corners = tile.getEdgePoly().getPoints();
    mids    = tile.getEdgePoly().getMids();

    unitMap = make_shared<Map>("Star2 unit map");
#if 1
    debugMap = make_shared<DebugMap>("Star2 debug map");
#endif

    QLineF branchRay = getRay(getN()-1,theta,false);

    QList<QPointF> isects;

    for (int side = 0; side <= clampRays(); ++side)
    {
        QPointF isect;
        QLineF otherRay = getRay(side,theta,true);
        if (branchRay.intersects(otherRay,&isect) == QLineF::BoundedIntersection)
        {
            isects.push_back(isect);
            if (debugMap) debugMap->insertDebugMark(isect,"");
        }
    }
    qDebug() << "intersections:" << isects.size();

    VertexPtr vt = unitMap->insertVertex(mids[getN()-1]);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for( int idx = 0; idx < isects.size() && idx < s; ++idx )
    {
        VertexPtr top = unitMap->insertVertex(isects[idx]);
        VertexPtr bot = unitMap->insertVertex(QPointF(isects[idx].x(), -isects[idx].y()));  // mirror

        unitMap->insertEdge(top_prev, top);
        unitMap->insertEdge(bot_prev, bot);

        top_prev = top;
        bot_prev = bot;
    }

    //unitMap->dumpMap(false);
    //unitMap->verify("Star::buildUnit");
}

QLineF Star2::getRay(int side, qreal theta, bool sign)
{
    //side = modulo(side,getN());

    qDebug() << "Ray:" << "n" << getN() << "side:" << side << "sign:" << sign << "theta" << theta;

    qreal angle;
    QLineF branchRay(mids[side], corners[side]);
    debugMap->insertDebugMark(mids[side],QString::number(side));
    if (sign)
    {
        angle = branchRay.angle();
        angle += theta;
    }
    else
    {
        angle = branchRay.angle() + 180.0;
        angle -= theta;
    }
    branchRay.setAngle(angle);

    qreal len = branchRay.length();
    len *= 100;
    branchRay.setLength(len);

    if (debugMap) debugMap->insertDebugLine(branchRay);

    return branchRay;
}

int Star2::clampRays()
{
    int a =  floor (0.5 * qreal(getN()) - 0.01);
    return qMin(s,a);
}

