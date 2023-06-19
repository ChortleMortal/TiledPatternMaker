#include "motifs/irregular_rosette.h"
#include "tile/tile.h"
#include "geometry/intersect.h"
#include "geometry/map.h"
#include "geometry/point.h"
#include "settings/configuration.h"


IrregularRosette::IrregularRosette() : IrregularMotif()
{
    setMotifType(MOTIF_TYPE_IRREGULAR_ROSETTE);
    setVersion(1);
}

void IrregularRosette::init(qreal q, qreal r, int s)
{
    this->q = q;
    this->r = r;
    this->s = s;
}

IrregularRosette::IrregularRosette(const Motif &other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_IRREGULAR_ROSETTE);
    try
    {
        auto rose = dynamic_cast<const IrregularRosette &>(other);
        debugSide   = rose.debugSide;
        mids        = rose.mids;
        corners     = rose.corners;
        branches    = rose.branches;
        center      = rose.center;
    }
    catch (std::bad_cast &)
    {
        return;
    }
}

void IrregularRosette::buildMotifMaps()
{
    Q_ASSERT(tile);
    motifMap = std::make_shared<Map>("IrregularRosette Map");
    inferRosette(tile);
    completeMotif(tile);
    completeMap();
    buildMotifBoundary(tile);
    buildExtendedBoundary();
}

//////////////////////////////////////////////////////////////////////////////
//
// Rosette inferring.
//
void IrregularRosette::inferRosette(TilePtr tile)
{
    corners = tile->getPoints();
    mids    = tile->getEdgePoly().getMids();
    center  = Point::center(mids);

    if (getVersion() == 2)
        buildV2(tile);
    else
        buildV1(tile);
}

void IrregularRosette::buildV1(TilePtr tile)
{
    qDebug() << "Infer::inferRosette  sides = " << tile->numSides();

    debugSide = 0;

#if 0
    debugMap = std::make_shared<DebugMap>("inferRosette debug map");
#endif

    mids.translate(-center);
    corners.translate(-center);

    branches.clear();
    int side_count = (Configuration::getInstance()->dontReplicate) ? 1 : mids.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qDebug() << "*** Build branch" << side;
        Branch branch  = buildRosetteBranchPointsV1(side, 1);
        branches.push_back(branch);
        branch         = buildRosetteBranchPointsV1(side, -1);
        branches.push_back(branch);
    }

    qDebug() << "num branches=" << branches.size();

    for (auto & branch : qAsConst(branches))
    {
        // update map
        motifMap->insertEdge(branch.tipPoint,branch.qePoint);
        QPolygonF intersections = buildRosetteIntersections(branch);
        int iCount = intersections.size();
        if (iCount)
        {
            qDebug() << "num intersections" << intersections.size();
            QPointF p1 = branch.qePoint;
            for (int is = 0; is < s && is < intersections.size(); ++is)
            {
                QPointF p2 = intersections[is];
                motifMap->insertEdge(p1,p2);
                p1 = p2;
            }
        }

        if (debugMap && branch.side == 0)
        {
            QString ssign = (branch.isign > 0) ? "+" : "-";
            debugMap->insertDebugMark(branch.tipPoint,QString("tipPt%1").arg(ssign));
            debugMap->insertDebugMark(branch.qePoint,QString("qePt%1").arg(ssign));
            debugMap->insertDebugMark(branch.fPoint,QString("fPt%1").arg(ssign));
            for (int i=0; i < intersections.size(); i++)
            {
                debugMap->insertDebugMark(intersections[i],QString("isect%1").arg(i));
            }
        }
    }
    //motifMap->cleanse(badEdges | badVertices_0 | badVertices_1);
    qDebug().noquote() << motifMap->namedSummary();
}

Branch IrregularRosette::buildRosetteBranchPointsV1(int side, int isign)
{
    qDebug() << "Build branch points" << side << isign;

    QPointF tip  = mids.get(side);                // The point to build from.
    QPointF rtip;
    if (isign >0)
        rtip = mids.next(side); // The next point over.
    else
        rtip = mids.prev(side); // The next point over.

    //DAC int s_clamp   = min( s, side_count / 2 );

    // Consider an equilateral triangle formed by the origin,
    // up_outer and a vertical edge extending down from up_outer.
    /// The center of the bottom edge of that triangle defines the
    // bisector of the angle leaving up_outer that we care about.

    QPointF down_outer = corners.get(side);
    QPointF up_outer   = corners.next(side);
    if (isign < 0)
    {
        std::swap(down_outer,up_outer);
    }

    QPointF bisector = down_outer * 0.5;

    QPointF e;
    if (!Intersect::getIntersection(up_outer, bisector, tip, rtip, e))
        e = Point::convexSum(up_outer, QPointF(0,0), 0.5 );

    QPointF ad;
    if (!Intersect::getIntersection(up_outer, bisector, tip, QPointF(0,0), ad))
        ad = QPointF(0,0);

    if (debugMap && side == debugSide)
    {
        QString ssign = (isign > 0) ? "+" : "-";

        debugMap->insertDebugLine(up_outer+center, bisector+center);
        debugMap->insertDebugLine(tip+center, rtip+center);
        debugMap->insertDebugLine(tip+center, QPointF()+center);

        debugMap->insertDebugMark(e,QString("e%1").arg(ssign));
        debugMap->insertDebugMark(ad,QString("ad%1").arg(ssign));
        debugMap->insertDebugMark(up_outer + center,QString("up_outer%1").arg(ssign));
        debugMap->insertDebugMark(down_outer+center,QString("down_outer%1").arg(ssign));
        debugMap->insertDebugMark(bisector + center,QString("bisector%1").arg(ssign));
        debugMap->insertDebugMark(tip  + center,QString("tip%1").arg(ssign));
        debugMap->insertDebugMark(rtip + center,QString("rtip%1").arg(ssign));
    }
    //QPointF qe = q_clamp >= 0  ? Point::convexSum(e, up_outer, q ) : Point::convexSum(e, ad, -q );
    QPointF qe;
    qreal q_clamp = std::min(std::max(q, -0.99),0.99);
    if (q_clamp >= 0)
    {
        qe = Point::convexSum(e, up_outer, q );
        //qe = Point::convexSum(e, ad, q );
    }
    else
    {
        qe = Point::convexSum(e, ad, -q );
    }

    QPointF f  = up_outer * r;

    Branch branch;
    branch.tipPoint = tip + center;
    branch.qePoint  = qe  + center;
    branch.fPoint   = f   + center;
    branch.side     = side;
    branch.isign    = isign;

    return branch;
}

void IrregularRosette::buildV2(TilePtr tile)
{
    qDebug() << "Infer::inferRosette  sides = " << tile->numSides();

    if (Loose::zero(q))
        q = 0.25;

 #if 1
    debugMap = std::make_shared<DebugMap>("inferRosette debug map");
#endif

    EdgePoly  ep      = tile->getEdgePoly();
    auto edges        = ep.getLines();

    qreal avgEdgeWidth = 0;
    for (int i=0; i < edges.size(); i++)
    {
        avgEdgeWidth += edges[i].length();
    }
    avgEdgeWidth /= edges.size();

    branches.clear();

    int side_count =  (Configuration::getInstance()->dontReplicate) ? 1 : edges.size();
    for (int side = 0; side < side_count; ++side )
    {
        Branch branch = buildRosetteBranchPointsV2(side, 1, avgEdgeWidth);
        branches.push_back(branch);
        branch = buildRosetteBranchPointsV2(side, -1,avgEdgeWidth);
        branches.push_back(branch);
    }
    qDebug() << "num branches=" << branches.size();

    for (auto & branch : qAsConst(branches))
    {
        // update map
        motifMap->insertEdge(branch.tipPoint,branch.qePoint);
        QPolygonF intersections = buildRosetteIntersections(branch);
        int iCount = intersections.size();
        if (iCount)
        {
            qDebug() << "num intersections" << intersections.size();
            QPointF p1 = branch.qePoint;
            for (int is = 0; is < s && is < intersections.size(); ++is)
            {
                QPointF p2 = intersections[is];
                motifMap->insertEdge(p1,p2);
                p1 = p2;
            }
        }

        if (debugMap && branch.side == 0)
        {
            QString ssign = (branch.isign > 0) ? "+" : "-";
            debugMap->insertDebugMark(branch.tipPoint,QString("tipPt%1").arg(ssign));
            debugMap->insertDebugMark(branch.qePoint,QString("qePt%1").arg(ssign));
            debugMap->insertDebugMark(branch.fPoint,QString("fPt%1").arg(ssign));
            for (int i=0; i < intersections.size(); i++)
            {
                debugMap->insertDebugMark(intersections[i],QString("isect%1").arg(i));
            }
        }
    }
    //motifMap->cleanse(badEdges | badVertices_0 | badVertices_1);
    qDebug().noquote() << motifMap->namedSummary();
}

Branch IrregularRosette::buildRosetteBranchPointsV2(int side, int isign, qreal sideLen)
{
    qreal maxLen = QLineF(corners[0],center).length();
    maxLen *= 1.5;

    qreal h = sideLen * q;
    qreal w = sideLen * q;

    // find mid p of base
    QLineF perp(mids[side],center);
    perp.setLength(h);
    QPointF base = perp.p2();

    if (debugMap && side==0)
        debugMap->insertDebugMark(base,"BASE");

    // create baseline
    QLineF bline;
    if (isign > 0)
        bline = QLineF(mids[side],corners.get(side));
    else
        bline = QLineF(mids[side],corners.next(side));
    bline.setLength(w);
    bline = Point::shiftParallel(bline,h);

    QPointF delta = base - bline.p2();
    bline.translate(delta);
    QPointF qe = bline.p1();

    // drop perpendicular (pLn) from qePt to edge containing tip and extend the line
    QPointF pPt= Point::perpPt(corners.get(side),corners.next(side), qe);
    QLineF pLine(pPt,qe);
    pLine.setLength(maxLen);
    pLine.setP1(qe);
    if (debugMap && side==0)
        debugMap->insertDebugLine(pLine);
    QPointF f = pLine.p2();

    Branch branch;
    branch.tipPoint = mids.get(side);
    branch.qePoint  = qe;
    branch.fPoint   = f;
    branch.side     = side;
    branch.isign    = isign;
    return branch;
}

Points IrregularRosette::buildRosetteIntersections(const Branch &branch)
{
    qDebug() << "Build branch intersections" << branch.side << branch.isign;

    qDebug() << mids.size();
    qDebug() << corners.size();

    UniqueQVector<QPointF> intersects;
    QPointF meet_f;
    for (int is = 1; (is <= s+1) && (intersects.size() < s); ++is)
    {
        int oside = modulo(branch.side + (branch.isign * is), mids.size());
        Branch obranch = findBranch(oside, -branch.isign);

        if (Intersect::getIntersection( branch.qePoint,  branch.qePoint + ( branch.fPoint -  branch.qePoint) * 10.0,
                                        obranch.qePoint, obranch.qePoint + (obranch.fPoint - obranch.qePoint) * 10.0, meet_f))
        {
            qDebug() << "isect side=" << branch.side << "sign=" << branch.isign << meet_f;
            intersects << meet_f;
        }
    }

    QMultiMap<qreal,QPointF> sortMap;
    for (int i=0; i < intersects.size(); i++)
    {
        sortMap.insert(QLineF(branch.qePoint,intersects[i]).length(), intersects[i]);
    }

    QVector<QPointF> intersections;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    intersections = sortMap.values();
#else
    QList<QPointF> list = sortMap.values();
    for (auto & pt : list)
    {
        intersections << pt;
    }
#endif
    return Points(intersections);
}

Branch & IrregularRosette::findBranch(int side, int sign)
{
    for (auto & branch : branches)
    {
        if (branch.side == side && branch.isign == sign)
            return branch;
    }
    qDebug() << "side" << side << "sign" << sign;
    qCritical() << "No branch found in" << branches.size() << "branches";
    return branches[0];
}

