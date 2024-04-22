#include <QtMath>
#include "irregular_star.h"
#include "geometry/map.h"
#include "geometry/intersect.h"
#include "geometry/vertex.h"
#include "tile/tile.h"

using std::make_shared;
using std::max;
using std::min;

IrregularStar::IrregularStar() : IrregularStarBranches()
{
    setMotifType(MOTIF_TYPE_IRREGULAR_STAR);
}

void IrregularStar::init(qreal d, int s)
{
    this->d = d;
    this->s = s;
}

IrregularStar::IrregularStar(const Motif &other) : IrregularStarBranches(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_STAR);
}

void IrregularStar::buildMotifMaps()
{
    Q_ASSERT(getTile());
    motifMap = make_shared<Map>("IrregularStar map");
    inferStar();
    scaleAndRotate();
    completeMap();
    buildMotifBoundary();
    buildExtendedBoundary();
}

void IrregularStar::inferStar()
{
    int ver = getVersion();

    qDebug() << "IrregularStar::inferStar version =" << ver;

    // qreal d  = angle (each integer is the next edge around the star)
    // int s    = number of intersects
    // qreal don = 1/n
    qDebug() << "n:" << getN() << "d:" << d << "s:" << s;

    if (ver == 3)
    {
        inferStarV3();
    }
    else if (ver == 2)
    {
        inferStarV2();
    }
    else
    {
        inferStarv1();
    }
}

void IrregularStar::inferStarv1()
{
    qDebug() << "Infer::inferStar";

    mids  = getTile()->getMids();

    int side_count = mids.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);

        motifMap->mergeMap( buildStarHalfBranchV1( d, s, side_frac, 1));
        motifMap->mergeMap( buildStarHalfBranchV1( d, s, side_frac, -1));
    }
    
    qDebug().noquote() << motifMap->summary();
}

void IrregularStar::inferStarV2()
{
#if 1
    debugMap = std::make_shared<DebugMap>("IrregularStarV1 debug map");
#endif

    corners = getTile()->getPoints();
    mids    = getTile()->getMids();
    if (corners.size() != getN())
    {
        qDebug() << "tile sides:" << corners.size() << "motif sides:" << getN();
        // need to interpolate mids and placed tile
    }

    for (int side = 0; side < getN(); ++side )
    {
        motifMap->mergeMap(buildStarHalfBranchV2(d, s, side, 1));
        motifMap->mergeMap(buildStarHalfBranchV2(d, s, side, -1));
    }
    
    qDebug().noquote() << motifMap->summary();
}

void IrregularStar::inferStarV3()
{
#if 1
    debugMap = std::make_shared<DebugMap>("IrregularStarV2 debug map");
#endif

    corners = getTile()->getPoints();
    mids    = getTile()->getMids();
    if (corners.size() != getN())
    {
        qDebug() << "tile sides:" << corners.size() << "motif sides:" << getN();
    }

    for (int side = 0; side < getN(); side++)
    {
        isects = getBranchIsectsV3(side,1);
        VertexPtr prv = motifMap->insertVertex(mids[side]);
        for (int i = 0; i < s && i < isects.size(); i++)
        {
            VertexPtr nxt = motifMap->insertVertex(isects[i]);
            motifMap->insertEdge(prv, nxt);
            prv = nxt;
        }
        isects = getBranchIsectsV3(side,-1);
        prv = motifMap->insertVertex(mids[side]);
        for (int i = 0; i < s && i < isects.size(); i++)
        {
            VertexPtr nxt = motifMap->insertVertex(isects[i]);
            motifMap->insertEdge(prv, nxt);
            prv = nxt;
        }
    }
    qDebug() << motifMap->summary();
}

QVector<QPointF> IrregularStar::getBranchIsectsV3(int side, int sign)
{
    isects.clear();

    int  di       = qFloor(d);          // di is equivalent to the mids index, equivalent to the edge index;
    qreal dfrac   = d - qreal(di);
    bool d_is_int = false;
    if (dfrac < Sys::TOL)
    {
        d_is_int = true;
    }
    else if ((1.0 - dfrac) < Sys::TOL)
    {
        d_is_int = true;
        di++;
    }

    qDebug() << "n:" << getN() << "d:" << d << "s:" << s << "di:" << di <<  "d_is_int:" << d_is_int;

    QLineF branchRay = getRay(side,d,sign);
    if (debugMap)
    {
        qreal angle = branchRay.angle();
        debugMap->insertDebugMark(branchRay.p1(),QString("a%1").arg(side));
        debugMap->insertDebugMark(branchRay.p2(),QString::number(angle));
        debugMap->insertDebugLine(branchRay);
    }

    for (int nside = 1; nside <= di; ++nside)
    {
        int theside;
        if (sign == 1)
            theside = modulo(side+nside,getN());
        else
            theside = modulo(side-nside,getN());

        QLineF otherRay = getRay(theside,d,-sign);
        if (debugMap)
        {
            qreal angle = otherRay.angle();
            debugMap->insertDebugMark(otherRay.p1(),QString("a%1").arg(theside));
            debugMap->insertDebugMark(otherRay.p2(),QString::number(angle));
            debugMap->insertDebugLine(otherRay);
        }
        QPointF isect;
        if (branchRay.intersects(otherRay,&isect) == QLineF::BoundedIntersection)
        {
            if (debugMap)
            {
                debugMap->insertDebugMark(isect,QString("isect%1").arg(theside));
            }
            isects.push_back(isect);
        }
    }

    if (s == di)
    {
        QTransform Tr  = QTransform().rotateRadians( 2.0 * M_PI / qreal(getN()));
        QPointF midr;
        if (!isects.isEmpty())
        {
            midr = Tr.map(isects.last());
        }
        else
        {
            midr = -center;
            midr = Tr.map(midr);
        }

        if (d_is_int)
        {
            isects.push_back(midr);
            if (debugMap)
            {
                debugMap->insertDebugMark(midr,"midr");
            }
        }
        else
        {
            QLineF arbr = getRay(di,d,-sign);
            QLineF mc   = getRay(side,d,sign);
            QPointF isect;
            bool rv = Intersect::getIntersection(arbr, mc,isect);
            if (debugMap)
            {
                debugMap->insertDebugMark(arbr.p1(),"ar");
                debugMap->insertDebugMark(arbr.p2(),"br");
                debugMap->insertDebugMark(mc.p2(),"c");
                debugMap->insertDebugLine(arbr);
                debugMap->insertDebugLine(mc);
            }
            if (rv)
            {
                isects.push_back(isect);
                if (debugMap)
                {
                    debugMap->insertDebugMark(isect,"isect");
                }
            }
        }
    }
    return isects;
}
