////////////////////////////////////////////////////////////////////////////
//
// Infer.java
//
// Infer is the black magic part of the Islamic design system.  Given
// an empty, irregular tile, it infers a map for that tile by
// extending line segments from neighbouring tiles.  For many common
// Islamic designs, this extension is natural and easy to carry out.
// Of course, because this design tool lets you explore designs that
// were never drawn in the Islamic tradition, there will be cases where
// it's not clear what to do.  There will, in fact, be cases where
// natural extensions of line segments don't exist.  So Infer has to
// have black magic built-in.  It has to use heuristics to construct
// plausible inferred maps, or at least maps that don't break the rest
// of the system.
//
// If you're using Taprats and experiencing weird behaviour, it's
// probably coming from in here.  But you knew what you were infer
// when you started using Taprats (sorry -- couldn't resist the pun).

#include <QDebug>

#include "motifs/inference_engine.h"
#include "motifs/motif.h"
#include "geometry/edge.h"
#include "geometry/intersect.h"
#include "geometry/loose.h"
#include "geometry/map.h"
#include "geometry/neighbours.h"
#include "geometry/point.h"
#include "geometry/transform.h"
#include "geometry/vertex.h"
#include "mosaic/prototype.h"
#include "settings/configuration.h"
#include "tile/tile.h"
#include "tile/placed_tile.h"
#include "tile/tiling.h"

using std::make_shared;
using std::max;
using std::min;

////////////////////////////////////////////////////////////////////////////
//
// Explicit figure inferring.
//
// Infer a map for the currently selected tile, using
// the app.Infer algorithm.  Absolutely definitely
// guaranteed to not necessarily work or produce satisfactory
// results.

InferenceEngine::InferenceEngine() : Motif()
{
    debug = false;
}

InferenceEngine::InferenceEngine(const Motif & fig) : Motif(fig)
{
    debug = false;
}

void InferenceEngine::setupInfer(PrototypePtr proto)
{
    tiling = proto->getTiling();
    if (!tiling)
    {
        qDebug() << "Infer::Infer = tiling is null";
        return;
    }

    qDebug().noquote() << "Infer::setup=" << proto.get()  << "tiling=" << tiling.get();



    // Get a map for each tile in the prototype.
    tileMaps.clear();
    QList<TilePtr> tiles = proto->getTiles();
    for (auto tile : tiles)
    {
        MotifPtr motif  = proto->getMotif(tile);
        Q_ASSERT(motif);
        qDebug().noquote() << "     motif=" <<  motif->getMotifDesc() << motif.get();
        tileMaps.insert(tile, motif->getMap());
    }

    // I'm going to generate all the tiles in the translational units
    // (x,y) where -1 <= x, y <= 1.  This is guaranteed to surround
    // every tile in the (0,0) unit by tiles.  You can then get
    // a sense of what other tiles surround a tile on every edge.
    allMotifMids.clear();
    for( int y = -1; y <= 1; ++y )
    {
        for( int x = -1; x <= 1; ++x )
        {
            // Building a 3x3 tiling of the prototype.
            // Create placed_points instances for all the tiles in the nine translational units
            QPointF pt   = (tiling->getData().getTrans1() * static_cast<qreal>(x)) + (tiling->getData().getTrans2() * static_cast<qreal>(y));
            QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

            const QVector<PlacedTilePtr> & pflist = tiling->getData().getPlacedTiles();
            for(auto pf : pflist)
            {
                QTransform Tf    = pf->getTransform() * T;
                TilePtr tile  = pf->getTile();

                QPolygonF fpts = Tf.map(tile->getPoints());

                QPolygonF featue_mids;
                int sz = tile->numPoints();
                for(int idx = 0; idx < sz; ++idx )
                {
                    featue_mids << Point::convexSum(fpts[idx], fpts[(idx+1)%sz], 0.5 );
                }

                allMotifMids << make_shared<MidPoints>(tile, Tf, featue_mids);
            }
        }
    }
}

MapPtr InferenceEngine::newExplicitMap()
{
    motifMap.reset();
    explicitMap = std::make_shared<Map>(sTileType[getMotifType()]);
    return explicitMap;
}

void InferenceEngine::inferStar(TilePtr tile, qreal d, int s)
{
    qDebug() << "Infer::inferStar";
    // Get the index of a good transform for this tile.
    int cur       = findPrimaryTile( tile );
    MidsPtr pmain = allMotifMids[cur];
    const QVector<QPointF> & mid_points  = pmain->getMidPoints();

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);

        explicitMap->mergeMap( buildStarHalfBranch( d, s, side_frac,  1.0, mid_points ));
        explicitMap->mergeMap( buildStarHalfBranch( d, s, side_frac, -1.0, mid_points ));
    }

    explicitMap->transformMap( pmain->T.inverted() );

    qDebug().noquote() << explicitMap->namedSummary();
}

void InferenceEngine::inferGirih(TilePtr tile, int starSides, qreal starSkip)
{
    qDebug() << "Infer::inferGirih";
    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal qstarSides           = static_cast<qreal>(starSides);
    qreal polygonInnerAngle    = M_PI * (qstarSides-2.0) / qstarSides;
    qreal starBranchInnerAngle = (starSkip * polygonInnerAngle) - ((starSkip - 1) * M_PI);
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

    // Get the index of a good transform for this tile.
    int cur       = findPrimaryTile(tile);
    MidsPtr pmain = allMotifMids[cur];
    const QVector<QPointF> & mid_points  = pmain->getMidPoints();

    QPolygonF points;
    for (int i = 0; i < mid_points.size(); ++i)
    {
        points <<  pmain->T.map(pmain->tile->getPoints()[i]);
    }

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        MapPtr branchMap = buildGirihBranch(side, requiredRotation, points, mid_points);
        explicitMap->mergeMap(branchMap);     // merge even if empty (null) map
    }

    explicitMap->transformMap( pmain->T.inverted() );
    qDebug().noquote() << explicitMap->namedSummary();
}

void InferenceEngine::inferIntersect(TilePtr tile, int starSides, qreal starSkip, int s)
{
    qDebug() << "Infer::inferIntersect";

    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal qstarSides           = static_cast<qreal>(starSides);
    qreal polygonInnerAngle    = M_PI * (qstarSides-2.0) / qstarSides;
    qreal starBranchInnerAngle = (starSkip * polygonInnerAngle) - (starSkip - 1) * M_PI;
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

    // Get the index of a good transform for this tile.
    int cur       = findPrimaryTile(tile);
    MidsPtr pmain = allMotifMids[cur];

    // Get the mid-points of each side of the tile.
    const QVector<QPointF> & mid_points  = pmain->getMidPoints();

    // Get the corners of the tiles.
    QPolygonF points;
    for (int i = 0; i < mid_points.size(); ++i)
    {
        points << pmain->T.map(pmain->tile->getPoints()[i]);
    }

    // Accumulate all edge intersections and their length.
    QList<EdgesLenPtr> infos;

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool isLeftHalf  = (i_halves == 0);
            QPointF sideHalf = buildGirihHalfBranch( side, isLeftHalf, requiredRotation, points, mid_points );
            QList<EdgesLenPtr> side_infos;
            side_infos = buildIntersectEdgesLengthInfos( side, sideHalf, isLeftHalf, requiredRotation, points, mid_points );
            infos += side_infos;
        }
    }

    // Sort edge-to-edge intersection by their total length.
#if 0
    Arrays.sort( infos, new Comparator<edges_length_info>()
    {
        public int compare( edges_length_info a, edges_length_info b )
                     { return a.compareTo( b ); } } );
#endif
    QMultiMap<qreal,EdgesLenPtr> sortMap;
    for (int i= 0; i < infos.size(); i++)
    {
        sortMap.insert(infos[i]->dist2,infos[i]);   // insert is sorted on key
    }
    infos = sortMap.values();

    // Record the starting point of each edge. As we grow the edge,
    // when we want more than one intersection (s > 1), we will update
    // these starting points.
    QPointF ** froms = new QPointF*[static_cast<unsigned int>(side_count)];
    for (int i=0; i<side_count; i++)
    {
        froms[i] = new QPointF[2];
    }

    for ( int i = 0; i < side_count; ++i )
    {
        froms[i][0] = mid_points[i];
        froms[i][1] = mid_points[i];
    }

    // Build the map using the shortest edges first.
    QVector<QVector<int>> counts(side_count,QVector<int>(2));    // thank you stack overflow
    for ( int i = 0; i < infos.size(); ++i )
    {
        EdgesLenPtr info = infos[i];
        int side1   = info->side1;
        int isLeft1 = info->isLeft1 ? 0 : 1;
        int side2   = info->side2;
        int isLeft2 = info->isLeft2 ? 0 : 1;
        if ( counts[side1][isLeft1] < s && counts[side2][isLeft2] < s )
        {
            QPointF from1 = froms[side1][isLeft1];
            QPointF from2 = froms[side2][isLeft2];
            VertexPtr m1  = explicitMap->insertVertex( from1 );
            VertexPtr m2  = explicitMap->insertVertex( from2 );
            if ( !info->intersection.isNull() )
            {
                VertexPtr inter = explicitMap->insertVertex( info->intersection );
                explicitMap->insertEdge( m1, inter );
                explicitMap->insertEdge( inter, m2 );
                froms[side1][isLeft1] = froms[side2][isLeft2] = info->intersection;
            }
            else
            {
                explicitMap->insertEdge( m1, m2 );
                froms[side1][isLeft1] = from2;
                froms[side2][isLeft2] = from1;
            }
            counts[side1][isLeft1] += 1;
            counts[side2][isLeft2] += 1;
        }
    }

    // added 21JUL21 clazy
    for ( int i = 0; i < side_count; ++i )
    {
        delete[] froms[i];
    }
    delete[] froms;

    explicitMap->transformMap( pmain->T.inverted() );
    qDebug().noquote() << explicitMap->namedSummary();
}

void InferenceEngine::inferIntersectProgressive(TilePtr tile, int starSides, qreal starSkip, int s)
{
    qDebug() << "Infer::inferIntersectProgressive";

    // We use the number of side of the star and how many side it
    // hops over from branch to branch (where 1 would mean drawing
    // a polygon) and deduce the inner angle of the star branches.
    // We support fractional side skipping.
    qreal qstarSides           = static_cast<qreal>(starSides);
    qreal polygonInnerAngle    = M_PI * (qstarSides-2.0) / qstarSides;
    qreal starBranchInnerAngle = (starSkip * polygonInnerAngle) - (starSkip - 1) * M_PI;
    qreal requiredRotation     = (M_PI - starBranchInnerAngle) / 2;

    // Get the index of a good transform for this tile.
    int cur       = findPrimaryTile(tile);
    MidsPtr pmain = allMotifMids[cur];

    // Get the mid-points of each side of the tile.
    const QVector<QPointF> & mid_points  = pmain->getMidPoints();

    // Get the corners of the tiles.
    QPolygonF points;
    for (int i = 0; i < mid_points.size(); ++i)
    {
        points << pmain->T.map(pmain->tile->getPoints()[i]);
    }

    int side_count = mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool isLeftHalf = (i_halves == 0);
            QPointF sideHalf = buildGirihHalfBranch( side, isLeftHalf, requiredRotation, points, mid_points );
            QList<IntersectionPtr> infos = buildIntersectionInfos(side, sideHalf, isLeftHalf, requiredRotation, points, mid_points);
            if ( !infos.isEmpty() )
            {
                // Record the starting point of the edge. As we grow the edge,
                // when we want more than one intersection (s > 1), we will update
                // this starting point.
                QPointF from = mid_points[side];

                // Build the map using the shortest edges first.
                int count = 0;
                for ( int i = 0; i < infos.size() && count < s; ++i )
                {
                    IntersectionPtr info = infos[i];
                    if ( !info->intersection.isNull())
                    {
                        VertexPtr p1  = explicitMap->insertVertex( from );
                        VertexPtr p2  = explicitMap->insertVertex( info->intersection );
                        explicitMap->insertEdge( p1, p2 );
                        from = info->intersection;
                        count++;
                    }
                }
            }
        }
    }

    explicitMap->transformMap(pmain->T.inverted());
    qDebug().noquote() << explicitMap->namedSummary();
}

// Hourglass inferring.
void InferenceEngine::inferHourglass(TilePtr tile, qreal d, int s)
{
    qDebug() << "Infer::inferHourglass";

    // Get the index of a good transform for this tile.
    int cur = findPrimaryTile(tile);
    MidsPtr pmain = allMotifMids[cur];
    const QVector<QPointF> & mid_points = pmain->getMidPoints();

    int side_count = mid_points.size();

    // Fix the s value to be between [0, side_count / 2 - 1]
    // instead of [1, side_count / 2].
    int side_modulo;
    if ( (side_count & 1) != 0 )
    {
        side_modulo = side_count;
    }
    else
    {
        side_modulo = side_count / 2;
    }
    s = s % side_modulo;

    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);
        qreal hour_d_pos = (side                 ) % side_modulo != s ? d : 1.0;
        qreal hour_d_neg = (side + side_count - 1) % side_modulo != s ? d : 1.0;
        explicitMap->mergeMap( buildStarHalfBranch( hour_d_pos, 1, side_frac,  1.0, mid_points ));
        explicitMap->mergeMap( buildStarHalfBranch( hour_d_neg, 1, side_frac, -1.0, mid_points ));
    }

    explicitMap->transformMap( pmain->T.inverted() );
    qDebug().noquote() << explicitMap->namedSummary();
}

//////////////////////////////////////////////////////////////////////////////
//
// Rosette inferring.
//
void InferenceEngine::inferRosette(TilePtr tile, qreal q, int s, qreal r)
{
    qDebug() << "Infer::inferRosette";

    debugSide = 0;

    debug = true;

    if (debug)
        debugMap = make_shared<DebugMap>("inferRosette debug map");

    // Get the index of a good transform for this tile.
    int cur         = findPrimaryTile(tile);
    MidsPtr pmain   = allMotifMids[cur];
    const QVector<QPointF> & mid_points = pmain->getMidPoints();

    QPolygonF corner_points = pmain->T.map(tile->getPoints());

    int side_count = (Configuration::getInstance()->dontReplicate) ? 1 : mid_points.size();
    for ( int side = 0; side < side_count; ++side )
    {
        qreal side_frac = static_cast<qreal>(side) / static_cast<qreal>(side_count);
        explicitMap->mergeMap(buildRosetteHalfBranch(side, q, s, r, side_frac,  1.0, mid_points, corner_points));
        explicitMap->mergeMap(buildRosetteHalfBranch(side, q, s, r, side_frac, -1.0, mid_points, corner_points));
    }

    explicitMap->transformMap(pmain->T.inverted());
    qDebug().noquote() << explicitMap->namedSummary();
}


MapPtr InferenceEngine::buildRosetteHalfBranch(int side, qreal q, int s, qreal r,
                                     qreal side_frac, qreal sign,
                                     QPolygonF mid_points,
                                     QPolygonF corner_points )
{
    Branch branch           = buildRosetteBranchPoints( side,q, s, r, side_frac, sign, mid_points, corner_points);
    QPolygonF intersections = buildRosetteIntersections(side, q, s, r, side_frac, sign, mid_points, corner_points, branch);

    if (debug && side == debugSide)
    {
        QString ssign = (sign > 0) ? "+" : "-";
        debugMap->insertDebugMark(branch.tipPoint,QString("tipPt%1").arg(ssign));
        debugMap->insertDebugMark(branch.qePoint,QString("qePt%1").arg(ssign));
        debugMap->insertDebugMark(branch.fPoint,QString("fPt%1").arg(ssign));
        for (int i=0; i < intersections.size(); i++)
        {
            debugMap->insertDebugMark(intersections[i],QString("isect%1%2").arg(i).arg(ssign));
        }
    }

    MapPtr branchMap = make_shared<Map>("infer buildRosetteHalfBranch map");
    branchMap->insertEdge(branch.tipPoint,branch.qePoint);
    QPointF p1 = branch.qePoint;
    for (int is = 0; is < s && is < intersections.size(); ++is)
    {
        QPointF p2 = intersections[is];
        branchMap->insertEdge(p1,p2);
        p1 = p2;
    }

    return branchMap;
}

// Rosette inferring.
Branch InferenceEngine::buildRosetteBranchPoints(int side, qreal q, int s, qreal r, qreal side_frac, qreal sign,
                                          QPolygonF mid_points, QPolygonF corner_points )
{
    Q_UNUSED(s)    // DAC
    int side_count = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);

    QPointF center = Point::center(mid_points);   // DAC - this algiorithm seems daffy
    mid_points.translate(-center);
    corner_points.translate(-Point::center(corner_points));

    QPointF tip  = getArc(side_frac, mid_points);                // The point to build from.
    QPointF rtip = getArc(side_frac + sign * circle_frac, mid_points); // The next point over.

    //DAC int s_clamp   = min( s, side_count / 2 );

    // Consider an equilateral triangle formed by the origin,
    // up_outer and a vertical edge extending down from up_outer.
    // The center of the bottom edge of that triangle defines the
    // bisector of the angle leaving up_outer that we care about.
    QPointF up_outer = getArc( side_frac + circle_frac, corner_points );
    QPointF down_outer = getArc( side_frac, corner_points );
    if ( sign < 0.0 )
    {
        QPointF temp = up_outer;
        up_outer = down_outer;
        down_outer = temp;
    }

    QPointF bisector = down_outer * 0.5;
    //bisector -= up_outer ;
    //bisector *= 10.0;
    //bisector += up_outer;

    QPointF e;
    if (!Intersect::getIntersection(up_outer, bisector, tip, rtip, e))
        e = Point::convexSum(up_outer, QPointF(0,0), 0.5 );

    QPointF ad;
    if (!Intersect::getIntersection(up_outer, bisector, tip, QPointF(0,0), ad))
        ad = QPointF(0,0);

    if (debug && side == debugSide)
    {
        QString ssign = (sign > 0) ? "+" : "-";

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
    qreal q_clamp = min(max(q, -0.99),0.99);
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

    //qDebug() << debugMap->namedSummary();

    return branch;
}

QPolygonF InferenceEngine::buildRosetteIntersections(int side, qreal q, int s, qreal r, qreal side_frac, qreal sign,
                                           QPolygonF mid_points, QPolygonF corner_points, Branch & branch )
{
    int side_count    = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);

    QPointF qe_f = branch.fPoint - branch.qePoint;
    QPolygonF intersections;

    //qreal other_sign = -1.0;
    for (qreal other_sign = -1.0; other_sign <= 1.0 + Loose::TOL; other_sign += 2.0)
    {
        for (int is = 1; (is <= s+1) && (intersections.size() < s); ++is)
        {
            Branch otherBranch = buildRosetteBranchPoints(side, q, s, r, side_frac + sign * circle_frac * is, other_sign * sign, mid_points, corner_points );
            QPointF other_qe_f = otherBranch.fPoint - otherBranch.qePoint;
            QPointF meet_f;
            if (Intersect::getIntersection(branch.qePoint, branch.qePoint + ( qe_f * 10.0), otherBranch.qePoint, otherBranch.qePoint + other_qe_f * 10.0, meet_f))
            {
                intersections << meet_f;
                qDebug() << "isect side=" << side << "sign=" << sign << meet_f;
            }
        }
    }

    QMultiMap<qreal,QPointF> sortMap;
    for (int i=0; i < intersections.size(); i++)
    {
        sortMap.insert(-Point::mag2(intersections[i]),intersections[i]);
    }
    QPolygonF ret = sortMap.values();
    return ret;
}

//////////////////////////////////////////////////////////////////////////////
//
// Normal (magic/simple) Infer
//
//////////////////////////////////////////////////////////////////////////////
void InferenceEngine::infer(TilePtr tile)
{
    enum eKind
    {
        // The different kinds of connectionJs that can be made between contacts, in increasing order of badness.
        // This is used to compare two possible connections.
        INSIDE_EVEN 	= 0,
        INSIDE_COLINEAR = 1,
        INSIDE_UNEVEN 	= 2,
        OUTSIDE_EVEN 	= 3,
        OUTSIDE_UNEVEN 	= 4,
        INFER_NONE 		= 5
    };

    qDebug() << "Infer::infer";

    // Get the index of a good transform for this tile.
    int cur        = findPrimaryTile( tile );
    MidsPtr pmain  = allMotifMids[ cur ];
    if (!pmain)
    {
        return;
    }
    QPolygonF fpts = pmain->T.map(pmain->tile->getPoints());
    qDebug() << "Infer fpts:" << fpts.size();

    // adjacencies
    QVector<AdjacencyPtr> adjacents = getAdjacencies(pmain, cur);

    // contacts
    QVector<ContactPtr> contacts = buildContacts(pmain, adjacents);

    qDebug() << "adjacencies=" << adjacents.size() << "contacts=" << contacts.size();
    if (contacts.size() == 0)
    {
        // this only works for PIC (polygons in contact)
        // DAC TODO - an approach would be to use the mid-points of the tile
        qWarning("No connections");
        return ;
    }

    // For every contact, if it hasn't found an extension,
    // Look at all other contacts for likely candidates.
    for (auto contact : contacts)
    {
        if (contact->taken)
        {
            continue;
        }

        ContactPtr bestOcon;
        QPointF    bestisect;
        qreal      bestdist = 0.0;
        eKind      bestkind = INFER_NONE;

        for (auto ocon : contacts)
        {
            if (ocon == contact)
            {
                continue;
            }

            if (ocon->taken)
            {
                continue;
            }

            // Don't try on two contacts that involve the same vertex.
            if (Loose::equalsPt(contact->position, ocon->position))
            {
                continue;
            }

            QPointF isect;
            qreal mydist = 0.0;
            eKind mykind = INFER_NONE;

            // First check if everybody's colinear.
            if( isColinear( contact->other, contact->position, ocon->position ) &&
                isColinear( contact->position, ocon->position, ocon->other ))
            {

                // The two segments have to point at each other.
                QPointF d1 = contact->position  - contact->other;
                QPointF d2 = ocon->position - ocon->other;
                QPointF dc = contact->position  - ocon->position;

                if( QPointF::dotProduct(d1,d2)> 0.0 )
                {
                    // They point in the same direction.
                    continue;
                }

                // The first segment must point at the second point.
                if( QPointF::dotProduct(d1,dc ) > 0.0 )
                {
                    continue;
                }

                if( QPointF::dotProduct(d2,dc ) < 0.0 )
                {
                    continue;
                }

                mykind = INSIDE_COLINEAR;
                mydist = Point::dist(contact->position,ocon->position);
            }
            else
            {
                if (Intersect::getTrueIntersection( contact->position, contact->end, ocon->position, ocon->end, isect))
                {
                    // We don't want the case where the intersection
                    // lies too close to either vertex.  Note that
                    // I think these checks are subsumed by
                    // getTrueIntersection.
                    if (Loose::equalsPt(isect, contact->position))
                    {
                        continue;
                    }
                    if (Loose::equalsPt(isect, ocon->position))
                    {
                        continue;
                    }

                    qreal dist  = Point::dist(contact->position,isect );
                    qreal odist = Point::dist(ocon->position,isect );

                    bool inside = fpts.containsPoint(isect,Qt::OddEvenFill);

                    if( !Loose::equals( dist, odist ) )
                    {
                        if( inside )
                        {
                            mykind = INSIDE_UNEVEN;
                        }
                        else
                        {
                            mykind = OUTSIDE_UNEVEN;
                        }
                        mydist = fabs( dist - odist );
                    }
                    else
                    {
                        if( inside )
                        {
                            mykind = INSIDE_EVEN;
                        }
                        else
                        {
                            mykind = OUTSIDE_EVEN;
                        }
                        mydist = dist;
                    }
                }
                else
                {
                    continue;
                }
            }

            if (lexCompareDistances( mykind, mydist, bestkind, bestdist ) < 0)
            {
                //qDebug() <<  "New best:"  << contact.get()  << ocon.get();
                bestOcon  = ocon;
                bestkind  = mykind;
                bestdist  = mydist;
                bestisect = isect;
            }
        }

        if (!bestOcon)
        {
            qDebug() << "Infer : Couldn't find a best match";
            continue;
        }

        auto ocon      = bestOcon;
        contact->taken = true;
        ocon->taken    = true;

        if( bestkind == INSIDE_COLINEAR )
        {
            qDebug() << "best is colinear";
            contact->colinear  = Contact::COLINEAR_MASTER;
            ocon->colinear = Contact::COLINEAR_SLAVE;
        }
        else
        {
            qDebug() << "isect: " << bestisect << contact.get() << ocon.get();
            contact->isect  = bestisect;
            ocon->isect = bestisect;
        }

        contact->isect_contact = bestOcon;
        ocon->isect_contact    = contact;
    }

    // Using the stored intersections in the contacts, build an inferred map.
    for (auto contact : contacts)
    {
        contact->dump();

        if (contact->isect.isNull())
        {
            if  (contact->colinear == Contact::COLINEAR_MASTER)
            {
                explicitMap->insertEdge(contact->position, contact->isect_contact->position );
                qDebug().noquote() << "Pass 1" << explicitMap->summary();
            }
        }
        else
        {
            explicitMap->insertEdge(contact->position, contact->isect );
            qDebug().noquote() << "Pass 1" << explicitMap->summary();
        }
    }

    // Try to link up unlinked edges.
    qreal minlen = Point::dist(fpts[0], fpts[fpts.size()-1] );
    for( int idx = 1; idx < fpts.size(); ++idx )
    {
        minlen = min( minlen, Point::dist(fpts[idx-1], fpts[ idx ] ) );
    }

    // DAC another pass where there is no isect
    for (auto con : contacts)
    {
        if (!con->isect_contact)
        {
            for (auto ocon : contacts )
            {
                if (ocon == con)
                {
                    continue;
                }
                if (ocon->isect_contact)
                {
                    continue;
                }

                // Two unmatched edges.  match them up.
                QPointF tmp  = con->position - con->other;
                tmp = Point::normalize(tmp);
                tmp *= (minlen*0.5);
                QPointF ex1 = con->position + tmp;  // DAC hard to decipher the java precedence rules used here

                tmp  = ocon->position - ocon->other;
                tmp = Point::normalize(tmp);
                tmp *= (minlen*0.5);
                QPointF ex2 = ocon->position + tmp; // ditto

                explicitMap->insertEdge( con->position, ex1 );
                explicitMap->insertEdge( ex1, ex2 );
                explicitMap->insertEdge( ex2, ocon->position );

                qDebug().noquote() << "Pass 2" << explicitMap->summary();

                con->isect_contact  = ocon;
                ocon->isect_contact = con;
            }
        }
    }

    explicitMap->transformMap(pmain->T.inverted());

    explicitMap->verify();

    if (explicitMap->isEmpty())
    {
        qWarning("Infer map: empty");
    }
    Q_ASSERT(!explicitMap->isEmpty());
    qDebug().noquote() << explicitMap->namedSummary();
}


void InferenceEngine::inferMotif(TilePtr tile)
{
    qDebug() << "Infer::inferMotif";

    EdgePoly epoly = tile->getEdgePoly();

    for (auto edge : epoly)
    {
        // this makes new eges and vertices since they can get altered in the map
        VertexPtr v1 = explicitMap->insertVertex(edge->v1->pt);
        VertexPtr v2 = explicitMap->insertVertex(edge->v2->pt);
        EdgePtr newEdge = explicitMap->insertEdge(v1,v2);
        if (edge->isCurve())
        {
            bool convex = edge->isConvex();
            QPointF ac  = edge->getArcCenter();
            newEdge->setArcCenter(ac,convex,(edge->getType() == EDGETYPE_CHORD));
        }
    }

    explicitMap->verify();
    qDebug().noquote() << explicitMap->namedSummary();
}

// Choose an appropriate transform of the tile to infer, i.e.
// one that is surrounded by other tiles.  That means that we
// should just find an instance of that tile in the (0,0) unit.
int InferenceEngine::findPrimaryTile(TilePtr tile )
{
    // The start and end of the tiles in the (0,0) unit.
    int start = tiling->getData().countPlacedTiles() * 4;
    int end   = tiling->getData().countPlacedTiles() * 5;
    int cur_reg_count = -1;
    int cur           = -1;

    for( int idx = start; idx < end; ++idx )
    {
        MidsPtr pp = allMotifMids[idx];

        if (pp->tile == tile)
        {
            // Count the number of regular tiles surrounding this one,
            // in the case a tile is not always surrounded by the same
            // tiles, we want to select the one with teh most regulars.
            QVector<AdjacencyPtr> adjs = getAdjacencies(pp, idx);
            if ( adjs.isEmpty() )
            {
                continue;
            }

            int new_reg_count = 0;
            for ( int i = 0; i < adjs.size(); i++ )
            {
                if ( adjs[i] != nullptr )
                {
                    if ( adjs[i]->tile->isRegular() )
                    {
                        new_reg_count++;
                    }
                }
            }
            if ( new_reg_count > cur_reg_count )
            {
                cur_reg_count = new_reg_count;
                cur = idx;
            }
        }
    }

    if( cur == -1 )
    {
        qWarning("Couldn't find tile in (0,0) unit!");
        cur = 0;
    }

    qDebug() << "Primary tile index =" << cur;
    return cur;
}

// For this edge of the tile being inferred, find the edges of neighbouring tiles and store.
AdjacencyPtr InferenceEngine::getAdjacency(QPointF main_point, int main_idx)
{
    static bool debug = false;

    if (debug) qDebug() << "Searching for adjacency for " << main_point;

    AdjacencyPtr ap;
    qreal tolerance = 1e-12;
    while (tolerance < 5.0)
    {
        for (int idx = 0; idx < allMotifMids.size(); ++idx)
        {
            if (idx == main_idx)
            {
                continue;
            }

            MidsPtr pcur = allMotifMids[idx];
            const QVector<QPointF> & mid_points  = pcur->getMidPoints();
            for (auto mid : mid_points)
            {
                if (Loose::Near(mid, main_point, tolerance))
                {
                    if (debug) qWarning() << "Found with tolerance " << tolerance ;
                    ap = make_shared<AdjacenycInfo>(pcur->tile, pcur->T, tolerance);
                    return ap;
                }
            }
        }
        tolerance *= 2;
    }

    return nullptr;
}

QVector<AdjacencyPtr> InferenceEngine::getAdjacencies(MidsPtr pp, int main_idx)
{
    QVector<AdjacencyPtr> ret;
    const QVector<QPointF> & mid_points  = pp->getMidPoints();
    for (auto pt: mid_points)
    {
        AdjacencyPtr ai = getAdjacency(pt, main_idx);
        if (ai)
        {
            ret.push_back(ai);
        }
    }
    return ret;
}

// Take the adjacencies and build a list of contacts by looking at vertices of the maps
// for the adjacent tiles that lie on the boundary with the inference region.
QVector<ContactPtr> InferenceEngine::buildContacts(MidsPtr pp, const QVector<AdjacencyPtr> & adjs)
{
    QVector<ContactPtr> contacts;

    QPolygonF tile_points = pp->T.map(pp->tile->getPoints());

    // Get the transformed map for each adjacent tile.  I'm surprised
    // at how fast this ends up being!
    QVector<MapPtr> adjacentMotifMaps;
    for (auto & adj : qAsConst(adjs))
    {
        MapPtr motifMap = tileMaps.value(adj->tile);
        if (motifMap)
        {
            MapPtr placedMotifMap = motifMap->recreate();
            placedMotifMap->transformMap(adj->T);
            adjacentMotifMaps.push_back(placedMotifMap);
            qDebug().noquote() << "adjacenct placed motif map" << placedMotifMap->summary();
        }
        else
        {
            adjacentMotifMaps.push_back(make_shared<Map>("Empty map"));    // DAC bugfix
            qDebug().noquote() << "adjacenct placed motif map - empty map";
        }
    }

    Q_ASSERT(adjacentMotifMaps.size());
    qDebug() << "adjacentMotifMaps" << adjacentMotifMaps.size();

    // Now, for each edge in the transformed tile, find a (hopefully _the_) vertex
    // in the adjacent map that lies on the edge.
    // When a vertex is found, all (hopefully _both_) the edges incident
    // on that vertex are added as contacts.

    for( int idx2 = 0; idx2 < tile_points.size(); ++idx2 )
    {
        QPointF a = tile_points[idx2];
        int  idx3 = (idx2+1) % tile_points.size();
        QPointF b = tile_points[idx3];

        if (a == b)
        {
            continue;   // DAC
        }

        MapPtr map       = adjacentMotifMaps[idx2];
        AdjacencyPtr adj = adjs[idx2];

        for (const auto & v : qAsConst(map->getVertices()))
        {
            QPointF pos = v->pt;
            qreal dist2 = Point::dist2ToLine(pos, a, b );
            if(         Loose::Near( dist2, adj->tolerance )
                  &&  ! Loose::Near( pos, a, adj->tolerance )
                  &&  ! Loose::Near( pos, b, adj->tolerance ) )
            {
                // This vertex lies on the edge.  Add all its edges to the contact list.
                NeighboursPtr n = map->getNeighbours(v);
                for (auto & wedge : *n)
                {
                    EdgePtr edge = wedge.lock();
                    QPointF opos  = edge->getOtherP(v);
                    contacts.push_back(make_shared<Contact>(pos, opos));
                }
            }
        }
    }
    return contacts;
}

// Pseudo points around a circle inscribed in the figure, like those for
// regular radial figures. Of course, the figure being ierrgular, we
// instead interpolate betwwen mid-points.
//
// XTODO: use bezier interpolation instead of linear.
QPointF InferenceEngine::getArc(qreal frac, const QPolygonF & pts )
{
    while ( frac > 1.0 )
        frac -= 1.0;
    while ( frac < 0.0 )
        frac += 1.0;
    int indexPrev = (static_cast<int>(floor(pts.size() * frac + 0.01))) % pts.size();
    int indexNext = (static_cast<int>(ceil( pts.size() * frac - 0.01))) % pts.size();
    QPointF prev = pts[ indexPrev ];
    QPointF next = pts[ indexNext ];

    qreal pts_frac = pts.size() * frac - indexPrev;
    return Point::convexSum( prev,next, pts_frac );
}

// Star inferring.
QPolygonF  InferenceEngine::buildStarBranchPoints( qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points)
{
    int side_count = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);

    qreal clamp_d = max( 1.0, min( d, 0.5 * side_count - 0.01 ) );

    int d_i = static_cast<int>(floor( clamp_d + 0.01));
    qreal d_frac = clamp_d - d_i;

    s = min( s, d_i );
    int outer_s = min( s, d_i - 1 );

    if( d_frac < Loose::TOL )
    {
        d_frac = 0.0;
    }
    else if( (1.0 - d_frac) < Loose::TOL )
    {
        d_frac = 0.0;
        d_i += 1;
    }

    QPointF a = getArc( side_frac, mid_points );
    QPointF b = getArc( side_frac + sign * clamp_d * circle_frac, mid_points );

    QPolygonF points;
    points << a;

    for( int idx = 1; idx <= outer_s; ++idx )
    {
        QPointF ar = getArc( side_frac + sign *  idx * circle_frac, mid_points );
        QPointF br = getArc( side_frac + sign * (idx - clamp_d) * circle_frac, mid_points );
        // FIXMECSK: we should handle the concave case by extending the intersection.
        //        (After all, two lines will intersect if not parallel and two
        //         consecutive edges can hardly be parallel.)
        QPointF inter;
        if (Intersect::getIntersection( a, b, ar, br, inter))
        {
            points << inter;
        }
    }

    return points;
}

MapPtr InferenceEngine::buildStarHalfBranch( qreal d, int s, qreal side_frac, qreal sign, QPolygonF mid_points )
{
    MapPtr map = make_shared<Map>("star half branch map");
    QPolygonF points = buildStarBranchPoints( d, s, side_frac, sign, mid_points );

    VertexPtr vt   = map->insertVertex( points[0] );
    VertexPtr prev = vt;

    for( int idx = 1; idx < points.size(); ++idx )
    {
        VertexPtr next = map->insertVertex( points[ idx ] );
        map->insertEdge( prev, next );
        prev = next;
    }

    int side_count    = mid_points.size();
    qreal circle_frac = 1.0 / static_cast<qreal>(side_count);
    qreal clamp_d     = max( 1.0, min( d, 0.5 * side_count - 0.01));
    int d_i           = static_cast<int>(floor(clamp_d + 0.01));
    qreal d_frac      = clamp_d - d_i;
    s                 = min( s, d_i );

    if( s == d_i && sign > 0 )
    {
        QPolygonF next_branch_points = buildStarBranchPoints( d, s, side_frac + sign * circle_frac, sign, mid_points );
        QPointF midr = next_branch_points[ next_branch_points.size() - 1 ];

        if ( d_frac == 0.0 )
        {
            VertexPtr v4 = map->insertVertex( midr );
            map->insertEdge( prev, v4 );
        }
        else
        {
            QPointF ar   = getArc( side_frac + sign * d_i    * circle_frac, mid_points );
            QPointF br   = getArc( side_frac - sign * d_frac * circle_frac, mid_points );
            QPointF c    = getArc( side_frac + sign * d * circle_frac, mid_points );
            QPointF cent;
            if (Intersect::getIntersection( ar, br, points[0], c, cent))
            {
                VertexPtr v4    = map->insertVertex( midr );
                VertexPtr vcent = map->insertVertex( cent );
                map->insertEdge( prev, vcent );
                map->insertEdge( vcent, v4 );
            }
        }
    }

    return map;
}

// Girih inferring.
QPointF InferenceEngine::buildGirihHalfBranch(int side, bool leftBranch, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    int side_count = points.size();
    int next = (side + 1) % side_count;
    QTransform rot = Transform::rotateAroundPoint(
                midPoints[side],
                leftBranch ? -requiredRotation : requiredRotation );

    QPointF halfBranch = rot.map( points[ leftBranch ? side : next ] );
    halfBranch -= midPoints[side];
    halfBranch *= 32;
    halfBranch += midPoints[side];

    return halfBranch;
}

IntersectionInfo InferenceEngine::findClosestIntersection(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    IntersectionInfo info;

    QPointF sideMidPoint = midPoints[side];

    bool otherIsLeft = !isLeftHalf;

    int side_count = points.size();
    for ( int i_side = 0; i_side < side_count; ++i_side )
    {
        if ( i_side == side )
            continue;

        QPointF otherMidPoint = midPoints[i_side];

        QPointF intersection;
        if ( Loose::zero(Point::dist2ToLine( otherMidPoint, sideMidPoint, sideHalf ) ) )
        {
            // Edge meets directly the other mid-points, so the distance is the shortest possible.
            intersection = Point::convexSum(otherMidPoint, sideMidPoint, 0.5 );
        }
        else
        {
            QPointF otherSide = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation, points, midPoints );
            if (!Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf, intersection))
            {
                continue;
            }
        }

        qreal dist2 = Point::dist2( intersection, sideMidPoint ) + Point::dist2(intersection, otherMidPoint );
        if ( Loose::equals( dist2, info.dist2 ) )
        {
            // In case of absolute equality, we use the nearest side.
            if ( abs(side - i_side) < abs(side - info.side) )
            {
                info.side = i_side;
                info.dist2 = dist2;
                info.intersection = intersection;
            }
        }
        if ( dist2 < info.dist2 )
        {
            info.side = i_side;
            info.dist2 = dist2;
            info.intersection = intersection;
        }
    }

    return info;
}

MapPtr InferenceEngine::buildGirihBranch(int side, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    MapPtr map = make_shared<Map>("buildGirihBranch map");

    // Mid-point of this side is always included in the map.
    VertexPtr midVertex  = map->insertVertex( midPoints[side] );

    // Find which other edge will intersect this one first.
    for ( int i_halves = 0; i_halves < 2; ++i_halves )
    {
        bool isLeftHalf = (i_halves == 0);

        // Find an intersection, if any.
        QPointF sideHalf       = buildGirihHalfBranch( side, isLeftHalf, requiredRotation, points, midPoints );
        IntersectionInfo info = findClosestIntersection( side, sideHalf, isLeftHalf, requiredRotation, points, midPoints );

        if ( info.intersection.isNull() )
            continue;

        VertexPtr interVertex = map->insertVertex( info.intersection );
        map->insertEdge( midVertex, interVertex );
    }

    return map;
}


// Intersect inferring.
int InferenceEngine::getIntersectionRank(int side, bool isLeft, QList<IntersectionPtr> infos)
{
    for ( int i = 0; i < infos.size(); ++i )
    {
        if ( infos[i]->otherSide == side && infos[i]->otherIsLeft == isLeft )
        {
            return i;
        }
    }
    return 100;
}

QList<EdgesLenPtr> InferenceEngine::buildIntersectEdgesLengthInfos(int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints)
{
    // First, get a list of intersections for this edge so that we can sort the
    // edge pairs by the fewest number of intersections.
    QList<IntersectionPtr> inter_infos = buildIntersectionInfos( side, sideHalf, isLeftHalf, requiredRotation, points, midPoints );

    QList<EdgesLenPtr> infos;

    QPointF sideMidPoint = midPoints[side];

    int side_count = points.size();
    for ( int i_side = side + 1; i_side < side_count; ++i_side )
    {
        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool otherIsLeft      = (i_halves == 0);
            QPointF otherMidPoint = midPoints[i_side];
            QPointF otherSide     = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation, points, midPoints );
            QList<IntersectionPtr> other_inter_infos = buildIntersectionInfos( i_side, otherSide, otherIsLeft, requiredRotation, points, midPoints );

            QPointF intersection;
            if (!Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf,intersection))
            {
                // Lines are parallel, see if they actually point at each other.
                if ( Loose::zero( Point::dist2ToLine(otherMidPoint, sideMidPoint, sideHalf ) ) )
                {
                    // Edge meets directly the other mid-points, so the distance is the middle in-between.
                    intersection = Point::convexSum( otherMidPoint, sideMidPoint, 0.5 );
                }
            }

            if (points.containsPoint(intersection,Qt::OddEvenFill))
            {
                int inter_rank = getIntersectionRank( i_side, otherIsLeft, inter_infos );
                int other_rank = getIntersectionRank( side, isLeftHalf, other_inter_infos );
                qreal dist2    = Point::dist2( intersection, sideMidPoint ) + Point::dist2( intersection, otherMidPoint );
                infos << make_shared<EdgesLengthInfo>(side, isLeftHalf, i_side, otherIsLeft, inter_rank + other_rank, dist2, intersection);
            }
        }
    }

    return infos;
}

// Progressive intersect inferring.
QList<IntersectionPtr> InferenceEngine::buildIntersectionInfos( int side, QPointF sideHalf, bool isLeftHalf, qreal requiredRotation, QPolygonF points, QPolygonF midPoints )
{
    Q_UNUSED(isLeftHalf)
    QList<IntersectionPtr> infos;

    QPointF sideMidPoint = midPoints[side];

    int side_count = points.size();
    for ( int i_side = 0; i_side < side_count; ++i_side )
    {
        if ( i_side == side )
            continue;

        for ( int i_halves = 0; i_halves < 2; ++i_halves )
        {
            bool otherIsLeft = (i_halves == 0);
            QPointF otherMidPoint = midPoints[i_side];

            QPointF otherSide = buildGirihHalfBranch( i_side, otherIsLeft, requiredRotation, points, midPoints );
            QPointF intersection;
            if (!Intersect::getIntersection( otherMidPoint, otherSide, sideMidPoint, sideHalf, intersection))
            {
                if ( Loose::zero( Point::dist2ToLine( otherMidPoint, sideMidPoint, sideHalf ) ) )
                {
                    // Edge meets directly the other mid-points, so the distance is the middle in-between.
                    intersection = Point::convexSum( otherMidPoint, sideMidPoint, 0.5 );
                }
            }
            qreal dist2 = Point::dist2(intersection, sideMidPoint );
            infos <<  make_shared<IntersectionInfo>(side, i_side, otherIsLeft, dist2, intersection);
        }
    }

    // Sort edge-to-edge intersection by their total length.
    QMultiMap<qreal,IntersectionPtr> sortMap;
    for (int i= 0; i < infos.size(); i++)
    {
        sortMap.insert(infos[i]->dist2,infos[i]);   // insert is sorted on key
    }
    infos = sortMap.values();

    return infos;
}

// "Normal" magic inferring.
bool InferenceEngine::isColinear( QPointF p, QPointF q, QPointF a )
{
    qreal px = p.x();
    qreal py = p.y();

    qreal qx = q.x();
    qreal qy = q.y();

    qreal x = a.x();
    qreal y = a.y();

    qreal left = (qx-px)*(y-py);
    qreal right = (qy-py)*(x-px);

    return Loose::equals( left, right );
}

int InferenceEngine::lexCompareDistances( int kind1, qreal dist1, int kind2, qreal dist2 )
{
    if( kind1 < kind2 )
    {
        return -1;
    }
    else if( kind1 > kind2 )
    {
        return 1;
    }
    else
    {
        if( Loose::equals( dist1, dist2 ) )
        {
            return 0;
        }
        else if( dist1 < dist2 )
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
}


// Transformed mid-points of a tile.
MidPoints::MidPoints(TilePtr tile, QTransform T, QVector<QPointF> & mids)
{
    this->tile = tile;
    this->midPoints    = mids;
    this->T       = T;
}

// Information about what tile and edge on that tile is adjacent to a given edge on a given tile.
AdjacenycInfo::AdjacenycInfo(TilePtr tile, QTransform T, qreal tolerance)
{
    this->tile   = tile;
    this->T         = T;
    this->tolerance = tolerance;
}

// Information about intersection of a side left or right in-going edge with a given side right or left edge.
IntersectionInfo::IntersectionInfo()
{
    side = -1;
    dist2 = 1e100;
    intersection = QPointF();
    Q_ASSERT(intersection.isNull());

    otherSide  = -1;
    otherIsLeft = false;
}

IntersectionInfo::IntersectionInfo( int side, int otherSide, bool otherIsLeft, qreal dist2, QPointF i )
{
    this->side         = side;
    this->otherSide    = otherSide;
    this->otherIsLeft  = otherIsLeft;
    this->dist2        = dist2;
    this->intersection = i;
}

bool  IntersectionInfo::equals( IntersectionInfo & other )
{
    qreal diff = dist2 - other.dist2;
    return diff > -Point::TOLERANCE2 && diff < Point::TOLERANCE2;
}

int  IntersectionInfo::compareTo(  IntersectionInfo & other )
{
    qreal diff = dist2 - other.dist2;
    return diff < -Point::TOLERANCE2 ? -1  : diff >  Point::TOLERANCE2 ?  1 : 0;
}

// Information about the length of edges connecting two sides and the intersection point.
EdgesLengthInfo::EdgesLengthInfo( int side1, bool isLeft1, int side2, bool isLeft2, int ic, qreal dist2, QPointF i )
{
    this->side1   = side1;
    this->isLeft1 = isLeft1;
    this->side2   = side2;
    this->isLeft2 = isLeft2;
    this->intersection_count = ic;
    this->dist2   = dist2;
    this->intersection = i;
}

bool EdgesLengthInfo::equals( EdgesLengthInfo & other )
{
    int ic_diff = intersection_count - other.intersection_count;
    if ( ic_diff != 0 )
        return false;

    qreal diff = dist2 - other.dist2;
    return diff > -Point::TOLERANCE2 && diff < Point::TOLERANCE2;
}

int EdgesLengthInfo::compareTo( EdgesLengthInfo & other )
{
    qreal diff = dist2 - other.dist2;
    if ( diff < -Point::TOLERANCE2 )
        return -1;
    if ( diff >  Point::TOLERANCE2 )
        return 1;

    int ic_diff = intersection_count - other.intersection_count;
    if ( ic_diff < 0 )
        return -1;
    else if ( ic_diff > 0 )
        return 1;

    return 0;
}

bool EdgesLengthInfo::lessThan(const EdgesLengthInfo* & e1, const EdgesLengthInfo*  & e2)
{
    return e1->dist2 < e2->dist2;
}

// The information about one point of contact on the boundary of the region being inferred.
Contact::Contact(QPointF position, QPointF other)
{
    this->position = position;
    this->other    = other;

    QPointF pt     = position - other;
    pt             = Point::normalize(pt);
    pt            *= 100.0;
    end            = position + pt;

    taken          = false;
    colinear       = COLINEAR_NONE;
}

