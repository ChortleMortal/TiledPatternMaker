#include <math.h>
#include "gui/viewers/motif_maker_view.h"
#include "model/motifs/adaptive_inference.h"
#include "model/motifs/adaptive_tools.h"
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

AdaptiveInference::AdaptiveInference() : AdaptiveMotif()
{
    qDebug() << "INFER CONSTRUCTOR";
    setMotifType(Type::Inferred);
    debugContacts = false;
}

AdaptiveInference::AdaptiveInference(const AdaptiveInference & other) : AdaptiveMotif(other)
{
    setMotifType(Type::Inferred);
    tiling       = other.tiling;
    allMotifMids = other.allMotifMids;
    debugContacts = false;
}

AdaptiveInference::AdaptiveInference(const Motif &other) : AdaptiveMotif(other)
{
    setMotifType(Type::Inferred);
    debugContacts = false;
    if (other.getMotifType() == Motif::Type::Inferred)
    {
        try
        {
            auto inf = dynamic_cast<const AdaptiveInference&>(other);
            tiling        = inf.tiling;
            allMotifMids  = inf.allMotifMids;
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

// This inferred motif type depends for its inference on the tiles surrounding this tile: it works
// by extending the already-built motif lines of NEIGHBORING tiles into this tile's empty interior
// and joining them up (see buildContacts()/evaluateCandidate() below). That's an elegant trick for
// a tile surrounded by enough symmetry - and matches how a lot of authentic Islamic patterns are
// actually constructed - but it causes a couple of real problems:
// 1. If an adjacent tile is another placement of the same motif then this affects the inference in unpredictable ways
// 2. If another adjacent tile is a different inferred motif, then the order in which the inferences are calculated
//    affects the outcome. Each inference affects the other.
// It may be for this reason that Kaplan chose to store irregular motifs as the calculated map (a blob), so in this way the
// vagaries of calculating inferences at load time are sidestepped.  However that makes it very difficult to
// reproduce any of the designs using the app from scratch where the released design had a blob.
//
// `handleEmptyMap` below is a workaround for problem 2, now narrowed to only the genuinely
// circular case: a non-Inferred neighbor is always forced to build on demand (see the
// getMotifMap()/getExistingMotifMap() branch just below), since it's always safe to do so and
// sidesteps any dependence on Prototype::buildMotifMaps()'s tile-iteration order. Only when the
// neighbor is ANOTHER AdaptiveInference - where forcing it to build risks it needing OUR
// not-yet-finished map right back - do we fall back to whatever's already been computed, and if
// that's still nothing, substitute a placeholder map built from that tile's own edge midpoints
// instead of its real motif. That lets buildContacts() find something to attach to on this pass
// rather than finding nothing, but it means the contacts harvested from a not-yet-inferred
// neighbor are approximate stand-ins, not the neighbor's real motif lines - so a ring of
// mutually-adjacent AdaptiveInference tiles will still not converge on the same answer a human
// tracing the same construction by hand would get.


void AdaptiveInference::infer()
{
    debugContacts       = false;
    bool debug          = false;
    bool handleEmptyMap = false;
    bool supportNotPIC  = false;     // PIC means Polygons in Contact

    if (debug) qDebug() << "InferredMotif::infer";

    ProtoPtr proto = wProto.lock();
    if (!proto)
    {
        qWarning() << "InferredMotif cannot infer - no valid Prototype";
        return;
    }

    // create the empty inferred map
    motifMap = std::make_shared<Map>("Inferred Motif map");

    if (debug) qDebug() << "InferredMotif::infer()  tile-sides :" << getTile()->numEdges();

    // Get the index of a good transform for this tile.
    int cur              = findPrimaryTile(getTile());
    if (debug) qDebug() << "primary feature index=" << cur;
    MidsPtr primaryMids  = allMotifMids[cur];
    Q_ASSERT(primaryMids);

    // adjacencies
    QVector<AdjacentTilePtr> adjacentTiles = getAdjacentTiles(primaryMids, cur);
    if (debug) qDebug() << "adjacent tiles =" << adjacentTiles.size();

    // Get a map for each motif in the prototype
    adjacentTileMaps.clear();
    for (const auto & adj : std::as_const(adjacentTiles))
    {
        TilePtr tile    = adj->placedTile->getTile();
        MotifPtr motif  = proto->getMotif(tile);
        Q_ASSERT(motif);
        if (debug) qDebug() << "Motif is:" <<  motif->getMotifTypeString();

        MapPtr adjmap;
        if (motif->getMotifType() == Motif::Type::Inferred)
        {
            // Another AdaptiveInference: forcing it to build here risks a genuine circular
            // dependency (it may in turn need OUR map, which isn't finished yet), so only take
            // whatever's already been computed - possibly nothing, if this is a true mutual
            // dependency or it just hasn't had its turn in Prototype::buildMotifMaps() yet.
            adjmap = motif->getExistingMotifMap();
        }
        else
        {
            // Every other motif type builds purely from its own tile's local geometry (see the
            // top-of-file comment), so it's always safe to force it to build right now rather
            // than accept a stale "not built yet" empty map caused only by tile-iteration order
            // in Prototype::buildMotifMaps()'s pass 1.
            adjmap = motif->getMotifMap();
        }

        if (handleEmptyMap && (!adjmap || adjmap->isEmpty()))
        {
            // this handles the case of more than one inferred motif in the mosaic's prototype
            auto mids = tile->getMids();
            adjmap = make_shared<Map>("mids map",mids);
        }

        if (debug)
        {
            if (adjmap)
                qDebug().noquote() << "     motif=" << adjmap->summary() << "Tile Sides" << tile->numEdges();
            else
                qDebug().noquote() << "     motif=" << "NO MAP" << "Tile Sides" << tile->numEdges();
        }

        adjacentTileMaps.insert(tile, adjmap);
    }
    if (debug) qDebug() << "adjacentTileMaps =" << adjacentTileMaps.size();

    // contacts
    QVector<ContactPtr> contacts = buildContacts(primaryMids, adjacentTiles);
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
            con->dump(idx++,"Contact");
        }
    }

    QPolygonF priPts = primaryMids->getTransformedPoints();
    if (debug) qDebug() << "Primary Points" << priPts.size() << priPts;

    // For every contact, if it hasn't found an extension,
    // Look at all other contacts for likely candidates.
    uint idx = 0;
    for (const auto & contact : std::as_const(contacts))
    {
        if (debugContacts) contact->dump(idx++,"processing contact");

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
            qreal   mydist = 0.0;
            eKind   mykind = INFER_NONE;

            if (!evaluateCandidate(contact, ocon, priPts, mykind, mydist, isect))
            {
                continue;
            }

            if (isBetterCandidate(mykind, mydist, bestkind, bestdist))
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

        if (debugContacts) qDebug().noquote() << "PAIRED edge" << contact->edgeIdx << "with edge" << ocon->edgeIdx << "kind" << sKind[bestkind] << "at" << bestisect;

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
                EdgePtr ep   = motifMap->insertEdge(v1,v2);
                // insertEdge() returns null for a zero-length (v1 == v2) edge - a matched pair
                // shouldn't ever produce one (see the evaluateCandidate() guards), but this is a
                // documented-nullable API, so don't blindly dereference it.
                if (ep)
                {
                    if (debugContacts) qDebug().noquote() << "Pass 1 inserting edge A:" << ep->info() << motifMap->summary();
                }
                else
                {
                    qWarning() << "Pass 1: skipped degenerate (zero-length) edge A";
                }
            }
        }
        else
        {
            VertexPtr v1 = motifMap->insertVertex(contact->position);
            VertexPtr v2 = motifMap->insertVertex(contact->isect);
            EdgePtr ep   = motifMap->insertEdge(v1,v2);
            if (ep)
            {
                if (debugContacts) qDebug().noquote() << "Pass 1 inserting edge B:" << ep->info() << motifMap->summary();
            }
            else
            {
                qWarning() << "Pass 1: skipped degenerate (zero-length) edge B";
            }
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
                    if (debug) qDebug().noquote() << "inserting edge:" << edge->info() << motifMap->summary();

                    edge = motifMap->insertEdge(vex1, vex2);
                    if (debug) qDebug().noquote() << "inserting edge:" << edge->info() << motifMap->summary();

                    edge = motifMap->insertEdge(vex2, vocon_position);
                    if (debug) qDebug().noquote() << "inserting edge:" << edge->info() << motifMap->summary();

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
        qWarning() << "Inferred Motif Map is empty";
    }
    
    if (debug) qDebug() << "InferredMotif::infer() - END" << motifMap->summary();
}

// Take the adjacencies and build a list of contacts by looking at vertices of the maps
// for the adjacent tiles that lie on the boundary with the inference region.
//
// One Contact is created per INCIDENT EDGE of each such boundary vertex, not one per tile edge:
// if a neighbor's motif line just passes straight through the boundary (the usual case), that
// vertex has two incident edges and produces two Contacts on this tile edge - one continuing
// each direction the line was heading. Both then need their own partner found independently in
// infer()'s matching loop below; they are not expected to pair with each other.
QVector<ContactPtr> AdaptiveInference::buildContacts(MidsPtr pp, const QVector<AdjacentTilePtr> & adjs)
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
                    ContactPtr c  = make_shared<Contact>(pos, opos);
                    c->edgeIdx    = iPoint;
                    contacts.push_back(c);
                }
            }
        }
    }

    if (debugContacts) qDebug() << "contacts =" << contacts.size();

    return contacts;
}

// True if p, q, a all lie on a common straight line.
bool AdaptiveInference::isColinear( QPointF p, QPointF q, QPointF a )
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

// Tests whether `contact` and `ocon` can be joined into one connection through this tile, and
// if so, how good a match it would be (out params kind/dist/isect). Returns false if there's no
// usable connection between them at all (e.g. their rays never cross).
//
// Two ways a pair can connect:
//  - COLINEAR: contact's and ocon's originating edges are already exactly colinear and pointing
//    at each other - they're really the two ends of what should be a single straight line, so
//    contact->position is joined directly to ocon->position with no new crossing point needed.
//  - INTERSECTING: contact and ocon each project a long ray into the tile (Contact::end - see
//    the Contact constructor), and those two rays cross at `isect`. How good that crossing is
//    depends on two questions:
//      - EVEN vs UNEVEN: is the crossing equidistant from contact and ocon? An even crossing
//        looks like a clean "V" and is preferred over a lopsided one.
//      - INSIDE vs OUTSIDE: does the crossing actually land inside the tile being inferred?
//        A crossing outside the tile means these two contacts don't really belong together,
//        but it's still accepted as a last resort if nothing better is available (see eKind).
bool AdaptiveInference::evaluateCandidate(const ContactPtr &contact, const ContactPtr &ocon, const QPolygonF &priPts,
                                           eKind &kind, qreal &dist, QPointF &isect)
{
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
            return false;
        }

        // The first segment must point at the second point.
        if( QPointF::dotProduct(d1,dc ) > 0.0 )
        {
            return false;
        }

        if( QPointF::dotProduct(d2,dc ) < 0.0 )
        {
            return false;
        }

        kind = INSIDE_COLINEAR;
        dist = Geo::dist(contact->position,ocon->position);
        return true;
    }

    if (Intersect::getTrueIntersection( contact->position, contact->end, ocon->position, ocon->end, isect))
    {
        if (debugContacts) qDebug() << "they intersect" << isect;

        // If the crossing point lands exactly on either contact's own starting position, this
        // isn't a real crossing - it's one ray passing through the other's start point - and
        // accepting it produces a zero-length edge later (Pass 1 builds contact->position to
        // isect) that Map::insertEdge rightly refuses, crashing the unchecked ep->info() call
        // above it. Must check both contact and ocon here, not just contact: with organic
        // motif-line geometry this coincidence is rare enough that only ever checking `contact`
        // went unnoticed, but the simple, highly-collinear placeholder ring `handleEmptyMap`
        // substitutes hits it easily.
        if (contact->position == isect || ocon->position == isect)
        {
            return false;
        }

        // We don't want the case where the intersection lies too close to either vertex.
        qreal cdist = Geo::dist(contact->position,isect );
        qreal odist = Geo::dist(ocon->position,isect );

#if defined(Q_OS_LINUX) || defined (Q_OS_MACOS)
        bool inside = Geo::pointInPolygon(isect,priPts);
#elif defined(Q_OS_WINDOWS)
        bool inside = priPts.containsPoint(isect,Qt::OddEvenFill);
#else
        Q_ASSERT(false);    // will not compile
#endif
        if (debugContacts) qDebug() << "inside" << inside;

        if( !Loose::equals(cdist, odist))
        {
            kind = (inside) ? INSIDE_UNEVEN : OUTSIDE_UNEVEN;
            dist = fabs(cdist - odist);
        }
        else
        {
            kind = (inside) ? INSIDE_EVEN : OUTSIDE_EVEN;
            dist = cdist;
        }
        return true;
    }

    if (debugContacts) qDebug() << "no intersection";
    return false;
}

// Ranks two connection candidates the way a dictionary orders words - "lexicographically",
// i.e. by (primary key, secondary key): kind is the primary key, and a lower eKind value always
// wins outright regardless of distance (see the eKind ordering comment). Only when both
// candidates have the exact same kind does the closer/more-even dist break the tie. This is
// how, among every still-available contact, we pick the single best partner for the contact
// currently being matched.
bool AdaptiveInference::isBetterCandidate(eKind kind1, qreal dist1, eKind kind2, qreal dist2 )
{
    if (kind1 != kind2)
    {
        return kind1 < kind2;
    }
    return dist1 < dist2 && !Loose::equals(dist1, dist2);
}

void AdaptiveInference::setupInfer(ProtoPtr proto)
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
    //
    // allMotifMids ends up as 9 consecutive blocks (one per (x,y) unit, in the y-then-x order
    // of the loop below), each block holding one TileMidPoints per tile in the prototype's unit.
    // (x,y)=(0,0) is therefore the 5th block, i.e. block index 4 - that's what findPrimaryTile()'s
    // "count * 4" is picking out below: the centre unit, fully surrounded by its 8 neighbors.
    allMotifMids.clear();
    for( int y = -1; y <= 1; ++y )
    {
        for( int x = -1; x <= 1; ++x )
        {
            // Building a 3x3 tiling of the prototype.
            // Create placed_points instances for all the tiles in the nine translational units
            QPointF pt   = (tiling->hdr().getTrans1() * static_cast<qreal>(x)) + (tiling->hdr().getTrans2() * static_cast<qreal>(y));
            QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

            const PlacedTileSet tileSet = tiling->unit().getIncluded();
            for (const auto & placedTile : std::as_const(tileSet))
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
int AdaptiveInference::findPrimaryTile(TilePtr tile )
{
    // The start and end of the tiles in the (0,0) unit - see the block-layout note in
    // setupInfer() above for why that's block 4 of the 9 built there.
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
            // tiles, we want to select the one with the most regular neighbors.
            QVector<AdjacentTilePtr> adjs = getAdjacentTiles(pp, idx);
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

    if (debugContacts) qDebug() << "Primary tile index =" << cur;
    return cur;
}

QVector<AdjacentTilePtr> AdaptiveInference::getAdjacentTiles(MidsPtr pp, int main_idx)
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
AdjacentTilePtr AdaptiveInference::getAdjacency(QPointF main_point, int main_idx)
{
    if (debugContacts) qDebug() << "Searching for adjacency for " << main_point;

    // Start with an exact-match tolerance and keep doubling it until some other tile's edge
    // midpoint matches main_point (or we give up at 5.0). The matched tolerance is stashed on
    // the returned AdjacentTile and later reused in buildContacts()'s own Loose::Near test, so
    // both places agree on how close is "close enough" for this particular pair of tiles.
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
                    ap = make_shared<AdjacentTile>(pcur->getPlacedTile(), tolerance);
                    return ap;
                }
            }
        }
        tolerance *= 2;
    }
    return ap;
}
