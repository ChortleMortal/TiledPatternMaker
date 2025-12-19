#include <math.h>
#include "gui/viewers/motif_maker_view.h"
#include "model/motifs/inferred_motif.h"
#include "model/motifs/irregular_tools.h"
#include "model/prototypes/prototype.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/intersect.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_verifier.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

#undef DEBUG_CONTACTS

InferredMotif::InferredMotif() : IrregularMotif()
{
    qDebug() << "INFER CONSTRUCTOR";
    setMotifType(MOTIF_TYPE_INFERRED);
    debugContacts = false;
}

InferredMotif::InferredMotif(const InferredMotif & other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_INFERRED);
    tiling       = other.tiling;
    allMotifMids = other.allMotifMids;
    debugContacts = false;
}

InferredMotif::InferredMotif(const Motif &other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_INFERRED);
    debugContacts = false;
    if (other.getMotifType() == MOTIF_TYPE_INFERRED)
    {
        try
        {
            auto inf = dynamic_cast<const InferredMotif&>(other);
            tiling        = inf.tiling;
            allMotifMids  = inf.allMotifMids;
        }
        catch(std::bad_cast &)
        {
            qWarning() << "Bad cast in IrregularInference constructor";
        }
    }
}

InferredMotif::InferredMotif(MotifPtr other) : IrregularMotif(other)
{
    setMotifType(MOTIF_TYPE_INFERRED);
    debugContacts = false;
    if (other->getMotifType() == MOTIF_TYPE_INFERRED)
    {
        try
        {
            auto inf     = std::dynamic_pointer_cast<InferredMotif>(other);
            tiling       = inf->tiling;
            allMotifMids = inf->allMotifMids;

        }
        catch(std::bad_cast &)
        {
            qWarning() << "Bad cast in IrregularInference constructor";
        }
    }
}

//////////////////////////////////////////////////////////
///
// Normal (magic/simple) Infer
///
//////////////////////////////////////////////////////////

// This inferred motif type depends for its inference on the tiles surrounding this tile
// This causes a number of problems:
// 1. If an adjacent tile is another placement of the same motif then this affects the inference in unpredictable ways
// 2. If another adjacent tile is a different inferred motif, then the order in which the inferences are calculated
//    affects the outcome. Each inference affects the other.
// It may be for this reason that Kaplan chose to store irregular motifs as the calculated map (a blob), so in this way the
// vagueries of calculating inferences at load time are sidestepped.  However that makes it very difficult to
// reproduce any of the designs using the app from scratch where the released design had a blob.


void InferredMotif::infer()
{
    qDebug() << "InferredMotif::infer";

#ifdef DEBUG_CONTACTS
    debugContacts  = true;
#endif
    bool handleEmptyMap = false;
    bool supportNotPIC  = false;     // PIC means Polygons in Contact

    ProtoPtr proto = wProto.lock();
    if (!proto)
    {
        qWarning() << "InferreMotif cannot infer - no valid Prototype";
        return;
    }

    // create the empty inferred map
    motifMap = std::make_shared<Map>("Inferred Motif map");

    qDebug() << "InferredMotif::infer()  tile-sides :" << getTile()->numEdges();

    // Get the index of a good transform for this tile.
    int cur              = findPrimaryTile(getTile());
    qDebug() << "primary feature index=" << cur;
    MidsPtr primaryMids  = allMotifMids[cur];
    Q_ASSERT(primaryMids);

    // adjacencies
    QVector<AdjacentTilePtr> adjacentTiles = getAdjacenctTiles(primaryMids, cur);
    qDebug() << "adjacenct tiles =" << adjacentTiles.size();

    // Get a map for each motif in the prototype
    adjacentTileMaps.clear();
    for (const auto & adj : std::as_const(adjacentTiles))
    {
        TilePtr tile    = adj->placedTile->getTile();
        MotifPtr motif  = proto->getMotif(tile);
        Q_ASSERT(motif);
        qDebug() << "Motif is:" <<  motif->getMotifTypeString();

        MapPtr adjmap = motif->getExistingMotifMap();

        if (handleEmptyMap)
        {
            if (!adjmap || adjmap->isEmpty())
            {
                // this handles the case of more than one inferred motif in the mosaic's prototype
                auto mids = tile->getMids();
                adjmap = make_shared<Map>("mids map",mids);
            }
            Q_ASSERT(adjmap);
            Q_ASSERT(!adjmap->isEmpty());
            qDebug().noquote() << "     motif=" << adjmap->summary() << "Tile Sides" << tile->numEdges();
        }
        else
        {
            if (adjmap)
                qDebug().noquote() << "     motif=" << adjmap->summary() << "Tile Sides" << tile->numEdges();
            else
                qDebug().noquote() << "     motif=" << "NO MAP" << "Tile Sides" << tile->numEdges();
        }

        adjacentTileMaps.insert(tile, adjmap);
    }
    qDebug() << "adjacentTileMaps =" << adjacentTileMaps.size();

    // contacts
    QVector<ContactPtr> contacts = buildContacts(primaryMids, adjacentTiles);
    qDebug() << "contacts =" << contacts.size();
    if (contacts.size() == 0)
    {
        // this only works for PIC (polygons in contact)
        // DAC TODO - an approach would be to use the mid-points of the tile
        qWarning("No contacts");
        return;
    }

    if (debugContacts)
    {
        uint idx = 0;
        debugContactPts = contacts;
        for (auto & con : contacts)
        {
            if (debugContacts) con->dump(idx++,"Contact");
        }
    }

    QPolygonF priPts = primaryMids->getTransformedPoints();
    qDebug() << "Primary Points" << priPts.size() << priPts;

    // For every contact, if it hasn't found an extension,
    // Look at all other contacts for likely candidates.
    uint idx = 0;
    for (const auto & contact : std::as_const(contacts))
    {
        if (debugContacts) contact->dump(idx++,"procressing contact");

        if (contact->taken)
        {
            if (debugContacts) qDebug() << "taken";
            continue;
        }

        ContactPtr bestocon;
        QPointF    bestisect;
        qreal      bestdist = 0.0;
        eKind      bestkind = INFER_NONE;

        int idx2 = 0;
        for (const auto &ocon : std::as_const(contacts))
        {
            if (debugContacts) ocon->dump(idx2++,"comparing other contact");

            if (ocon == contact)
            {
                if (debugContacts) qDebug() << "same contact";
                continue;
            }

            if (ocon->taken)
            {
                if (debugContacts) qDebug() << "taken";
                continue;
            }

            // Don't try on two contacts that involve the same vertex.
            if (Loose::equalsPt(contact->position, ocon->position))
            {
                if (debugContacts) qDebug() << "same contact point";
                continue;
            }

            QPointF isect;
            qreal mydist = 0.0;
            eKind mykind = INFER_NONE;

            // First check if everybody's colinear.
            if( isColinear( contact->other, contact->position, ocon->position ) &&
                isColinear( contact->position, ocon->position, ocon->other ))
            {
                if (debugContacts) qDebug() << "are colinear";
                // The two segments have to point at each other.
                QPointF d1 = contact->position  - contact->other;
                QPointF d2 = ocon->position     - ocon->other;
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
                mydist = Geo::dist(contact->position,ocon->position);
            }
            else if (Intersect::getTrueIntersection( contact->position, contact->end, ocon->position, ocon->end, isect))
            {
                if (debugContacts) qDebug() << "they intersect" << isect;

                if (contact->position == isect)
                {
                    continue;
                }

                // We don't want the case where the intersection lies too close to either vertex.
                qreal dist  = Geo::dist(contact->position,isect );
                qreal odist = Geo::dist(ocon->position,isect );

#if defined(Q_OS_LINUX) || defined (Q_OS_MACOS)
                bool inside = Geo::pointInPolygon(isect,priPts);
#elif defined(Q_OS_WINDOWS)
                bool inside = priPts.containsPoint(isect,Qt::OddEvenFill);
#else
                Q_ASSERT(false);    // will not compile
#endif
                if (debugContacts) qDebug() << "inside" << inside;

                if( !Loose::equals(dist, odist))
                {
                    mykind = (inside) ? INSIDE_UNEVEN : OUTSIDE_UNEVEN;
                    mydist = fabs(dist - odist);
                }
                else
                {
                    mykind = (inside) ? INSIDE_EVEN : OUTSIDE_EVEN;
                    mydist = dist;
                }
            }
            else
            {
                if (debugContacts) qDebug() << "no intersection";
                continue;
            }

            if (lexCompareDistances(mykind, mydist, bestkind, bestdist) < 0)
            {
                if (debugContacts) qDebug() <<  "New best:"; //  << contact.get()  << ocon.get();
                bestocon  = ocon;
                bestkind  = mykind;
                bestdist  = mydist;
                bestisect = isect;
            }
        }

        if (!bestocon)
        {
            qDebug() << "InferredMotif::infer : Couldn't find a best match";
            continue;
        }

        auto ocon      = bestocon;
        contact->taken = true;
        ocon->taken    = true;

        if( bestkind == INSIDE_COLINEAR )
        {
            if (debugContacts) qDebug() << "best is colinear";
            contact->colinear  = Contact::COLINEAR_MASTER;
            ocon->colinear     = Contact::COLINEAR_SLAVE;
        }
        else
        {
            if (debugContacts) qDebug() << "isect: " << bestisect; // << contact.get() << ocon.get();
            contact->isect  = bestisect;
            ocon->isect     = bestisect;
        }

        contact->isect_contact = bestocon;
        ocon->isect_contact    = contact;
    }

    // Using the stored intersections in the contacts, build an inferred map.
    idx = 0;
    for (const auto & contact : std::as_const(contacts))
    {
        if (debugContacts) contact->dump(idx++,"Using contact");
        if (contact->isect.isNull())
        {
            if (contact->colinear == Contact::COLINEAR_MASTER)
            {
                VertexPtr v1 = motifMap->insertVertex(contact->position);
                VertexPtr v2 = motifMap->insertVertex(contact->isect_contact.lock()->position);
                EdgePtr ep = motifMap->insertEdge(v1,v2);
                if (debugContacts) qDebug().noquote() << "Pass 1 inserting edge A:" << ep->info() << motifMap->summary();
            }
        }
        else
        {
            VertexPtr v1 = motifMap->insertVertex(contact->position);
            VertexPtr v2 = motifMap->insertVertex(contact->isect);
            EdgePtr ep   = motifMap->insertEdge(v1,v2);
            if (debugContacts) qDebug().noquote() << "Pass 1 inserting edge B:" << ep->info() << motifMap->summary();
        }
    }

    if (supportNotPIC)
    {
        // Try to link up unlinked edges.
        qreal minlen = Geo::dist(priPts[0], priPts[priPts.size()-1] );
        for( int idx = 1; idx < priPts.size(); ++idx )
        {
            minlen = std::min( minlen, Geo::dist(priPts[idx-1], priPts[ idx ] ) );
        }

        // DAC another pass where there is no isect because polygons are not in contact
        for (const auto &con : std::as_const(contacts))
        {
            if (!con->isect_contact.lock())
            {
                for (auto & ocon : contacts )
                {
                    if (ocon == con)
                    {
                        continue;
                    }
                    if (ocon->isect_contact.lock())
                    {
                        continue;
                    }

                    // Two unmatched edges.  match them up.
                    QPointF tmp  = con->position - con->other;
                    tmp = Geo::normalize(tmp);
                    tmp *= (minlen*0.5);
                    QPointF ex1 = con->position + tmp;  // DAC hard to decipher the java precedence rules used here

                    tmp  = ocon->position - ocon->other;
                    tmp = Geo::normalize(tmp);
                    tmp *= (minlen*0.5);
                    QPointF ex2 = ocon->position + tmp; // ditto

                    VertexPtr vcon_position  = motifMap->insertVertex(con->position);
                    VertexPtr vocon_position = motifMap->insertVertex(ocon->position);
                    VertexPtr vex1           = motifMap->insertVertex(ex1);
                    VertexPtr vex2           = motifMap->insertVertex(ex2);

                    EdgePtr  edge = motifMap->insertEdge(vcon_position, vex1);
                    qDebug().noquote() << "inserting edge:" << edge->info() << motifMap->summary();

                    edge = motifMap->insertEdge(vex1, vex2);
                    qDebug().noquote() << "inserting edge:" << edge->info() << motifMap->summary();

                    edge = motifMap->insertEdge(vex2, vocon_position);
                    qDebug().noquote() << "inserting edge:" << edge->info() << motifMap->summary();

                    con->isect_contact  = ocon;
                    ocon->isect_contact = con;
                }
            }
        }
    }

    motifMap->transform(primaryMids->getTransform().inverted());

    MapVerifier mv(motifMap);
    mv.verify();

    if (motifMap->isEmpty())
    {
        qWarning() << "Inferred Motif Map is emtpty";
    }
    
    qDebug() << "InferredMotif::infer() - END" << motifMap->summary();
}

// Take the adjacencies and build a list of contacts by looking at vertices of the maps
// for the adjacent tiles that lie on the boundary with the inference region.
QVector<ContactPtr> InferredMotif::buildContacts(MidsPtr pp, const QVector<AdjacentTilePtr> & adjs)
{
    QPolygonF tile_points = pp->getTransformedPoints();
    if (debugContacts) qDebug() << "tile_points" << tile_points;

    // Get the transformed map for each adjacent tile.  I'm surprised
    // at how fast this ends up being!
    QVector<MapPtr> adjacentMotifMaps;
    for (const auto & adj : std::as_const(adjs))
    {
        MapPtr motifMap = adjacentTileMaps.value(adj->placedTile->getTile());
        if (motifMap)
        {
            if (debugContacts) qDebug().noquote() << "Existing adjacent map"  << motifMap->summary();
            MapPtr placedMotifMap = motifMap->recreate();
            placedMotifMap->transform(adj->placedTile->getPlacement());
            adjacentMotifMaps.push_back(placedMotifMap);
            if (debugContacts) qDebug().noquote() << "Placed   adjacent map" << placedMotifMap->summary();
        }
        else
        {
            adjacentMotifMaps.push_back(make_shared<Map>("Empty map"));    // DAC bugfix
            if (debugContacts) qDebug().noquote() << "Placed   adjacent map - EMPTY";
        }
    }

    if (debugContacts) qDebug() << "adjacent motif maps:" << adjacentMotifMaps.size();
    Q_ASSERT(adjacentMotifMaps.size());

    // Now, for each edge in the transformed tile, find a (hopefully _the_) vertex
    // in the adjacent map that lies on the edge.
    // When a vertex is found, all (hopefully _both_) the edges incident
    // on that vertex are added as contacts.

    QVector<ContactPtr> contacts;
    for (int iPoint = 0; iPoint < tile_points.size(); ++iPoint )
    {
        QPointF pt         = tile_points[iPoint];
        int     iNextPoint = (iPoint+1) % tile_points.size();
        QPointF nextPt     = tile_points[iNextPoint];

        if (pt == nextPt)
        {
            continue;   // DAC
        }

        MapPtr          map = adjacentMotifMaps[iPoint];
        AdjacentTilePtr adj = adjs[iPoint];
        NeighbourMap   nmap(map);

        for (const auto & v : std::as_const(map->getVertices()))
        {
            QPointF pos = v->pt;
            qreal dist2 = Geo::dist2ToLine(pos, pt, nextPt);
            if (Loose::Near(dist2, adj->tolerance) &&  !Loose::Near(pos, pt, adj->tolerance) &&  !Loose::Near(pos, nextPt, adj->tolerance))
            {
                // This vertex lies on the edge.  Add all its edges to the contact list.
                NeighboursPtr n = nmap.getNeighbours(v);
                for (auto & wedge : std::as_const(*n))
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

bool InferredMotif::isColinear( QPointF p, QPointF q, QPointF a )
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

#define E2STR(x) #x

const QString  sKind[] =
{
    E2STR(INSIDE_EVEN),
    E2STR(INSIDE_COLINEAR),
    E2STR(INSIDE_UNEVEN),
    E2STR(OUTSIDE_EVEN),
    E2STR(OUTSIDE_UNEVEN),
    E2STR(INFER_NONE),
};

int InferredMotif::lexCompareDistances(eKind kind1, qreal dist1, eKind kind2, qreal dist2 )
{
    if (debugContacts) qDebug().noquote() << "lex" << sKind[kind1] << sKind[kind2];

    int rv = 1;
    if (kind1 < kind2)
    {
        rv = -1;
        if (debugContacts) qDebug() << "lex 1  better kind rv =" << rv;
    }
    else if (kind1 > kind2)
    {
        rv = 1;
        if (debugContacts) qDebug() << "lex 2 worse kind rv =" << rv;
    }
    else if (Loose::equals(dist1, dist2))
    {
        rv =  0;
        if (debugContacts) qDebug() << "lex 3 equal distances rv =" << rv;
    }
    else if (dist1 < dist2)
    {
        rv = -1;
        if (debugContacts) qDebug() << "lex 4 better distance" << dist1 << dist2 << "rv =" << rv;
    }
    else
    {
        rv = 1;
        if (debugContacts) qDebug() << "lex 5 worse distance rv =" << rv;
    }

    return rv;
}

void InferredMotif::setupInfer(ProtoPtr proto)
{
    wProto = proto;

    tiling = proto->getTiling();
    if (!tiling)
    {
        qDebug() << "Infer::Infer = tiling is null";
        return;
    }

    qDebug().noquote() << "Infer::setup=" << proto.get()  << "tiling=" << tiling.get();

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
            QPointF pt   = (tiling->hdr().getTrans1() * static_cast<qreal>(x)) + (tiling->hdr().getTrans2() * static_cast<qreal>(y));
            QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

            const PlacedTiles tilingUnit = tiling->unit().getIncluded();
            for (const auto & placedTile : std::as_const(tilingUnit))
            {
                QTransform Tf   = placedTile->getPlacement() * T;
                TilePtr tile    = placedTile->getTile();

                QPolygonF t_pts = Tf.map(tile->getPoints());
                EdgePoly t_ep   = tile->getEdgePoly().map(Tf);

                Points tileMidpoints;
                int sz = tile->numPoints();
                for(int idx = 0; idx < sz; ++idx )
                {
                    tileMidpoints << Geo::convexSum(t_pts[idx], t_pts[(idx+1)%sz], 0.5 );
                }

                allMotifMids << make_shared<TileMidPoints>(make_shared<PlacedTile>(tile,Tf), tileMidpoints, t_ep);
            }
        }
    }
}

// Choose an appropriate transform of the tile to infer, i.e.
// one that is surrounded by other tiles.  That means that we
// should just find an instance of that tile in the (0,0) unit.
int InferredMotif::findPrimaryTile(TilePtr tile )
{
    // The start and end of the tiles in the (0,0) unit.
    int count = tiling->unit().numIncluded();
    int start = count * 4;
    int end   = count * 5;
    int cur_reg_count = -1;
    int cur           = -1;

    for( int idx = start; idx < end; ++idx )
    {
        MidsPtr pp = allMotifMids[idx];

        if (pp->getTile() == tile)
        {
            // Count the number of regular tiles surrounding this one,
            // in the case a tile is not always surrounded by the same
            // tiles, we want to select the one with teh most regulars.
            QVector<AdjacentTilePtr> adjs = getAdjacenctTiles(pp, idx);
            if ( adjs.isEmpty() )
            {
                continue;
            }

            int new_reg_count = 0;
            for ( int i = 0; i < adjs.size(); i++ )
            {
                if ( adjs[i] != nullptr )
                {
                    if ( adjs[i]->placedTile->getTile()->isRegular() )
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

QVector<AdjacentTilePtr> InferredMotif::getAdjacenctTiles(MidsPtr pp, int main_idx)
{
    QVector<AdjacentTilePtr> ret;
    const QVector<QPointF> & mid_points  = pp->getTileMidPoints();
    for (auto & pt : std::as_const(mid_points))
    {
        AdjacentTilePtr ai = getAdjacency(pt, main_idx);
        if (ai)
        {
            ret.push_back(ai);
        }
    }
    return ret;
}

// For this edge of the tile being inferred, find the edges of neighbouring tiles and store.
AdjacentTilePtr InferredMotif::getAdjacency(QPointF main_point, int main_idx)
{
    if (debugContacts) qDebug() << "Searching for adjacency for " << main_point;

    AdjacentTilePtr ap;
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
            const QVector<QPointF> & mid_points  = pcur->getTileMidPoints();
            for (auto & mid : std::as_const(mid_points))
            {
                if (Loose::Near(mid, main_point, tolerance))
                {
                    if (debugContacts) qDebug() << "Found with tolerance " << tolerance ;
                    ap = make_shared<AdjacenctTile>(pcur->getPlacedTile(), tolerance);
                    return ap;
                }
            }
        }
        tolerance *= 2;
    }
    return ap;
}
