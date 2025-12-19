#include <QtMath>
#include "irregular_star.h"
#include "sys/geometry/map.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/vertex.h"
#include "model/tilings/tile.h"
#include "gui/viewers/debug_view.h"

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

void IrregularStar::infer()
{
    int ver = getVersion();
    //qDebug() << "IrregularStar::infer" << "version =" << ver;

    motifMap = make_shared<Map>("IrregularStar map");

    // qreal d  = angle (each integer is the next edge around the star)
    // int s    = number of intersects
    // qreal don = 1/n
    //qDebug() << "n:" << getN() << "d:" << d << "s:" << s;

    if (motifDebug)
    {
        Sys::debugMapCreate->wipeout();
    }

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
    //qDebug() << "IrregularStar::inferStarv1";

    mids  = getTile()->getMids();

    int side_count = mids.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);

        motifMap->mergeMap( buildStarHalfBranchV1( d, s, side_frac, 1));
        motifMap->mergeMap( buildStarHalfBranchV1( d, s, side_frac, -1));
    }
    
    //().noquote() << motifMap->summary();
}

void IrregularStar::inferStarV2()
{
    //qDebug() << "IrregularStar::inferStarV2";

    corners = getTile()->getPoints();
    mids    = getTile()->getMids();
    if (corners.size() != getN())
    {
        qWarning() << "tile sides:" << corners.size() << "motif sides:" << getN();
        // need to interpolate mids and placed tile
    }

    for (int side = 0; side < getN(); ++side )
    {
        motifMap->mergeMap(buildStarHalfBranchV2(d, s, side, 1));
        motifMap->mergeMap(buildStarHalfBranchV2(d, s, side, -1));
    }
    
    //qDebug().noquote() << motifMap->summary();
}

void IrregularStar::inferStarV3()
{
    //qDebug() << "IrregularStar::inferStarV3";

    corners = getTile()->getPoints();
    mids    = getTile()->getMids();
    if (corners.size() != getN())
    {
        qWarning() << "tile sides:" << corners.size() << "motif sides:" << getN();
    }

    for (int side = 0; side < getN(); side++)
    {
        isects = getBranchIsectsV3(side,1);
        VertexPtr prv = motifMap->insertVertex(mids[side]);
        for (int i = 0; i < s && i < isects.size(); i++)
        {
            VertexPtr nxt = motifMap->insertVertex(isects[i]);
            motifMap->insertEdge(prv,nxt);
            prv = nxt;
        }
        isects = getBranchIsectsV3(side,-1);
        prv = motifMap->insertVertex(mids[side]);
        for (int i = 0; i < s && i < isects.size(); i++)
        {
            VertexPtr nxt = motifMap->insertVertex(isects[i]);
            motifMap->insertEdge(prv,nxt);
            prv = nxt;
        }
    }
    //qDebug() << motifMap->summary();
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

    //qDebug() << "n:" << getN() << "d:" << d << "s:" << s << "di:" << di <<  "d_is_int:" << d_is_int;

    QLineF branchRay = getRay(side,d,sign);
    if (motifDebug)
    {
        qreal angle = branchRay.angle();
        Sys::debugMapCreate->insertDebugMark(branchRay.p1(),QString("a%1").arg(side),Qt::red);
        Sys::debugMapCreate->insertDebugMark(branchRay.p2(),QString::number(angle),Qt::red);
    }

    for (int nside = 1; nside <= di; ++nside)
    {
        int theside;
        if (sign == 1)
            theside = modulo(side+nside,getN());
        else
            theside = modulo(side-nside,getN());

        QLineF otherRay = getRay(theside,d,-sign);
        if (motifDebug)
        {
            qreal angle = otherRay.angle();
            Sys::debugMapCreate->insertDebugMark(otherRay.p1(),QString("a%1").arg(theside),Qt::red);
            Sys::debugMapCreate->insertDebugMark(otherRay.p2(),QString::number(angle),Qt::red);
            Sys::debugMapCreate->insertDebugLine(otherRay,Qt::blue);
        }
        QPointF isect;
        if (branchRay.intersects(otherRay,&isect) == QLineF::BoundedIntersection)
        {
            if (motifDebug)
            {
                Sys::debugMapCreate->insertDebugMark(isect,QString("isect%1").arg(theside),Qt::red);
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
            if (motifDebug)
            {
                Sys::debugMapCreate->insertDebugMark(midr,"midr",Qt::red);
            }
        }
        else
        {
            QLineF arbr = getRay(di,d,-sign);
            QLineF mc   = getRay(side,d,sign);
            QPointF isect;
            bool rv = Intersect::getIntersection(arbr, mc,isect);
            if (motifDebug)
            {
                Sys::debugMapCreate->insertDebugMark(arbr.p1(),"ar",Qt::red);
                Sys::debugMapCreate->insertDebugMark(arbr.p2(),"br",Qt::red);
                Sys::debugMapCreate->insertDebugMark(mc.p2(),"c",Qt::red);
                Sys::debugMapCreate->insertDebugLine(arbr,Qt::blue);
                Sys::debugMapCreate->insertDebugLine(mc,Qt::blue);
            }
            if (rv)
            {
                isects.push_back(isect);
                if (motifDebug)
                {
                    Sys::debugMapCreate->insertDebugMark(isect,"isect",Qt::red);
                }
            }
        }
    }
    return isects;
}
