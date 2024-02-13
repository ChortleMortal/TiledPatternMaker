////////////////////////////////////////////////////////////////////////////
//
// Rosette2
//
// The rosette is a classic feature of Islamic art.  It's a star
// shape surrounded by hexagons.
//
// This class implements the rosette as a RadialFigure using the
// geometric construction given in Lee [1].
//
// [1] A.J. Lee, _Islamic Star Patterns_.  Muqarnas 4.
//
// Rosette2 is an alternative to Kaplan's Rosette, which relaxes
// Kaplans constraint on Knee position which is only a partial truth
//

#include <QDebug>
#include "motifs/rosette2.h"
#include "geometry/edge.h"
#include "geometry/geo.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "geometry/map.h"
#include "tile/tile.h"

typedef std::shared_ptr<Rosette2> Rosette2Ptr;

using std::make_shared;

#define DBG_VAL 0x15

Rosette2::Rosette2(const Motif & fig,  int nsides, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c) : RadialMotif(fig, nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    k           = kk;
    constrain   = c;
    tipType     = TIP_TYPE_OUTER;
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

Rosette2::Rosette2(int nsides, qreal kneeX, qreal kneeY, int ss, qreal kk, bool c) : RadialMotif(nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    k           = kk;
    constrain   = c;
    tipType     = TIP_TYPE_OUTER;
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

Rosette2::Rosette2(const Rosette2 & other) : RadialMotif(other)
{
    kneeX       = other.kneeX;
    kneeY       = other.kneeY;
    s           = other.s;
    k           = other.k;
    constrain   = other.constrain;
    tipType     = other.tipType;
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

bool Rosette2::equals(const MotifPtr other)
{
    Rosette2Ptr otherp = std::dynamic_pointer_cast<Rosette2>(other);
    if (!otherp)
        return  false;

    if (kneeX != otherp->kneeX)
        return  false;

    if (kneeY != otherp->kneeY)
        return  false;

    if (k != otherp->k)
        return false;

    if (s != otherp->s)
        return false;

    if (tipType != otherp->tipType)
        return false;

    if (!Motif::equals(other))
        return false;

     return true;
}

bool Rosette2::pointOnLineLessThan(QPointF p1, QPointF p2)
{
    return  Geo::dist2(kneePt,p1) < Geo::dist2(kneePt,p2);
}

// kneeX is the %age of the distance to the center, so is always scaled by 1.0
// kneey is the %age of half the edge width, so scaaling is depends on n (sides)
void Rosette2::buildUnitMap()
{
    if ((DBG_VAL > 0) && !debugMap)
        debugMap = make_shared<DebugMap>("rosette debug map");
    else
        debugMap.reset();

    qDebug().noquote() << "Rosette::buildUnit n:" << getN() << "x:" << kneeX << "y:" << kneeY << "s:" << s << "k" << k
                       << "Tr:" << Transform::toInfoString(radialRotationTr) << "rot" << getMotifRotate();

    Tile atile(getN());
    xRange = 1.0;
    yRange = atile.edgeLen() /2.0;

    qreal x = xRange * kneeX;
    qreal y = yRange * kneeY;

    kneePt = QPointF(1.0 - x, y);

    if (constrain)
    {
        calcConstraintLine();
        kneePt = Geo::getClosestPoint(constraint,kneePt);
    }

    QPointF kneePtMirror(kneePt.x(),-kneePt.y());

    QPointF edgePt = Geo::perpPt(atile.getEdge(getN()-1),kneePt);

    QLineF  ray(edgePt,kneePt);         // ray from tile to knee point
    ray.setLength(2.0);                 // extend ray
    ray.setP1(kneePt);                  // the real ray
    ray.setAngle(ray.angle() +k );      // factors in knee angle (k)

    QLineF rayMirror(kneePtMirror, QPointF(ray.p2().x(), -ray.p2().y()));

    QPointF outerTip(1.0, 0.0);                                 // standard tip

    if (DBG_VAL & 0x01)
    {
        debugMap->insertDebugMark(kneePt,"kneePt");
        debugMap->insertDebugLine(ray);
        debugMap->insertDebugLine(rayMirror);
    }

    UniqueQVector<QPointF> epoints;

    epoints.push_back(kneePt);

    for (int idx = 0; idx <= getN(); idx++)
    {
        rayMirror = radialRotationTr.map(rayMirror);

        QPointF isect;
        if (ray.intersects(rayMirror,&isect) == QLineF::BoundedIntersection)
        {
            epoints.push_back(isect);
        }
    }

    std::sort(epoints.begin(), epoints.end(), [this] (QPointF a, QPointF b) { return pointOnLineLessThan(a, b); });

    if (DBG_VAL & 0x04)
    {
        qDebug() << "epoints" << epoints;
        int idx = 0;
        for (auto & ept :  std::as_const(epoints))
        {
            debugMap->insertDebugMark(ept,QString("pt%1").arg(idx++));
        }
    }

    // fill the map
    unitMap = make_shared<Map>("rosette unit map");
    if (tipType ==  TIP_TYPE_OUTER)
    {
        buildSegement(unitMap,outerTip,epoints);
    }
    else
    {
        QLineF  kneeLine(kneePt,kneePtMirror);
        QPointF innerTip = Geo::reflectPoint(outerTip,kneeLine);    // outer tip reflected over kneeLine

        if (tipType == TIP_TYPE_INNER)
        {
            buildSegement(unitMap,innerTip,epoints);
        }
        else
        {
            Q_ASSERT(tipType == TIP_TYPE_ALTERNATE);
            buildSegement(unitMap,outerTip,epoints);
            unitMap2 = make_shared<Map>("rosette unit map2");
            buildSegement(unitMap2,innerTip,epoints);
            //unitMap2->transformMap(Tr);
        }
    }

}

void  Rosette2::buildSegement(MapPtr map, QPointF tip, QList<QPointF> & epoints)
{
    VertexPtr vt       = map->insertVertex(tip);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for (int idx = 0; idx < epoints.size() && idx <= s; ++idx)
    {
        VertexPtr top = map->insertVertex(epoints[idx]);
        VertexPtr bot = map->insertVertex(QPointF(epoints[idx].x(), - epoints[idx].y()));

        map->insertEdge(top_prev, top);
        map->insertEdge(bot_prev, bot);

        top_prev = top;
        bot_prev = bot;
    }
}

void Rosette2::replicate()
{
    if (tipType == TIP_TYPE_OUTER || tipType == TIP_TYPE_INNER)
    {
        RadialMotif::replicate();
        return;
    }

    Q_ASSERT(tipType == TIP_TYPE_ALTERNATE);
    
    QTransform T = radialRotationTr;       // rotaional transform
    motifMap = make_shared<Map>("Rosett2 map");

    for( int idx = 0; idx < getN(); ++idx)
    {
        MapPtr uMap = (idx & 1) ? unitMap : unitMap2;
        for (auto & edge : std::as_const(uMap->getEdges()))
        {
            QPointF v1 = T.map(edge->v1->pt);
            QPointF v2 = T.map(edge->v2->pt);
            VertexPtr vp1 = motifMap->insertVertex(v1);
            VertexPtr vp2 = motifMap->insertVertex(v2);
            motifMap->insertEdge(vp1,vp2);
        }
        T *= radialRotationTr;
    }

    motifMap->verify();
}

void Rosette2::calcConstraintLine()
{
    qreal   don         = get_don();
    qreal   qr_outer    = 1.0 / qCos( M_PI * don );
    QPointF r_outer     = QPointF(0.0, qr_outer);
    QPointF up_outer    = getArc( 0.5 * don ) * qr_outer;
    QPointF down_outer  = up_outer - r_outer;
    QPointF bisector    = down_outer * 0.5;

    constraint  = QLineF(bisector, up_outer);

    if (DBG_VAL > 0x10)
    {
        debugMap->insertDebugLine(constraint);
        debugMap->insertDebugMark(up_outer,"up_outer");
        debugMap->insertDebugMark(bisector,"bisector");
    }
}

bool Rosette2::convertConstrained()
{
    if (!constrain) return false;

    qreal  x    = kneePt.x() / xRange;
    qreal  y    = kneePt.y() / yRange;

    kneeX       = 1.0 - x;
    kneeY       = y;
    constrain   = false;

    return true;
}
