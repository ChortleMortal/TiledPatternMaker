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

Rosette2::Rosette2(const Motif & fig,  int nsides, qreal kneeX, qreal kneeY, int ss) : RadialMotif(fig, nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    tipType     = TIP_TYPE_OUTER;
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

Rosette2::Rosette2(int nsides, qreal kneeX, qreal kneeY, int ss) : RadialMotif(nsides)
{
    this->kneeX = kneeX;
    this->kneeY = kneeY;
    s           = ss;
    tipType     = TIP_TYPE_OUTER;
    setMotifType(MOTIF_TYPE_ROSETTE2);
}

Rosette2::Rosette2(const Rosette2 & other) : RadialMotif(other)
{
    kneeX       = other.kneeX;
    kneeY       = other.kneeY;
    s           = other.s;
    tipType     = other.tipType;
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

    if (s != otherp->s)
        return false;

    if (tipType != otherp->tipType)
        return false;

    if (!Motif::equals(other))
        return false;

     return true;
}

// kneeX is the %age of the distance to the center, so is always scaled by 1.0
// kneey is the %age of half the edge width, so scaaling is depends on n (sides)
void Rosette2::buildUnitMap()
{
#if 1
    debugMap = make_shared<DebugMap>("rosette debug map");
#endif

    int dbgVal = 0x06;

    qDebug().noquote() << "Rosette::buildUnit n:" << getN() << "x:" <<kneeX << "y" << kneeY << "s" << s
                       << "Tr:" << Transform::toInfoString(radialRotationTr) << "rot" << getMotifRotate();

    Tile atile(getN());
    qreal yRange = atile.edgeLen() /2.0;
    qreal xRange = 1.0;

    qreal y = yRange * kneeY;
    qreal x = xRange * kneeX;
    QPointF kneePt(1.0 - x, y);
    QPointF kneePtMirror(kneePt.x(),-kneePt.y());
    
    QPointF edgePt = Geo::perpPt(atile.getEdge(getN()-1),kneePt);
    QLineF  line(edgePt,kneePt);
    line.setLength(line.length()*100.0);
    QPointF kneeEndPt = line.p2();
    QPointF kneeEndPtMirror(kneeEndPt.x(), -kneeEndPt.y());

    QLineF kneeLine(kneePt,kneePtMirror);
    QPointF tip(1.0, 0.0);         // The point to build from
    QPointF tipMirror = Geo::reflectPoint(tip,kneeLine);

    if (debugMap && (dbgVal & 0x04))
    {
        debugMap->insertDebugMark(edgePt,"edgePt");
        debugMap->insertDebugMark(kneePt,"kneePt");
        debugMap->insertDebugMark(kneePtMirror,"kneePtMirror");
        debugMap->insertDebugMark(kneeEndPtMirror,"kneeEndPtMirror");

        debugMap->insertDebugLine(kneePt,kneeEndPt);
        debugMap->insertDebugLine(kneePtMirror,kneeEndPtMirror);
    }

    QList<QPointF> epoints;
    epoints.push_back(kneePt);
    for (int idx = 1; idx <= s; ++idx )
    {
        kneePtMirror    = radialRotationTr.map(kneePtMirror);
        kneeEndPtMirror = radialRotationTr.map(kneeEndPtMirror);

        QLineF keyLine( kneePt, kneeEndPt);
        QLineF keyRLine(kneePtMirror,kneeEndPtMirror);

        QPointF isect;
        if (keyLine.intersects(keyRLine,&isect) == QLineF::BoundedIntersection)
        {
            epoints.push_back(isect);
            if (debugMap && idx == 1 && (dbgVal & 0x08))
            {
                debugMap->insertDebugLine(kneePtMirror,kneeEndPtMirror);
                debugMap->insertDebugMark(kneePtMirror,QString("key_r_point%1").arg(idx));
                debugMap->insertDebugMark(kneeEndPtMirror,QString("key_r_end%1").arg(idx));
                debugMap->insertDebugMark(isect,QString("isect%1").arg(idx));
            }
        }
    }

    //QList<QPointF> epoints2 = epoints;
    //epoints[0]              = kneePtMirror;

    // fill the map
    unitMap = make_shared<Map>("rosette unit map");
    switch (tipType)
    {
    case TIP_TYPE_OUTER:
        buildSegement(unitMap,tip,epoints);
        break;

    case TIP_TYPE_INNER:
        buildSegement(unitMap,tipMirror,epoints);
        break;

    case TIP_TYPE_ALTERNATE:
        buildSegement(unitMap,tip,epoints);
        unitMap2 = make_shared<Map>("rosette unit map2");
        buildSegement(unitMap2,tipMirror,epoints);
       // unitMap2->transformMap(Tr);
    }
}

void  Rosette2::buildSegement(MapPtr map, QPointF tip, QList<QPointF> & epoints)
{
    VertexPtr vt       = map->insertVertex(tip);
    VertexPtr top_prev = vt;
    VertexPtr bot_prev = vt;

    for (int idx = 0; idx < epoints.size(); ++idx)
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
