#include <math.h>
#include "motifs/inferred_motif.h"
#include "motifs/irregular_tools.h"
#include "tile/tile.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"
#include "geometry/map.h"
#include "makers/prototype_maker/prototype.h"
#include "tile/tiling.h"
#include "viewers/motif_view.h"

using std::make_shared;

#undef DEBUG_CONTACTS
#undef HANDLE_EMPTY_MAP

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

void InferredMotif::buildMotifMaps()
{
    qDebug() << "InferredMotif::buildMotifMaps";
    Q_ASSERT(proto.lock());
    Q_ASSERT(tile);
    infer(proto.lock(),tile);
    completeMotif(tile);
    completeMap();
    buildMotifBoundary(tile);
    buildExtendedBoundary();
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


void InferredMotif::infer(ProtoPtr proto, TilePtr tile)
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

#ifdef DEBUG_CONTACTS
    debugContacts = true;
#endif

    qDebug() << "InferredMotif::infer()  tile-sides :" << tile->numSides();

    qDebug() << "Get a map for each motif in the prototype.";
    adjacentTileMaps.clear();
    QList<TilePtr> tiles = proto->getTiles();
    for (const auto & tile : tiles)
    {
        MotifPtr motif  = proto->getMotif(tile);
        Q_ASSERT(motif);
        qDebug() << "Motif is:" <<  motif->getMotifTypeString();

        MapPtr map;
        if (motif.get() != this)
        {
            map = motif->getMotifMap(); // the getMap builds maps as needed on demand
        }

#ifdef HANDLE_EMPTY_MAP
        if (!map || map->isEmpty())
        {
            // this handles the case of more than one inferred motif in the mosaic's prototype
            auto poly = tile->getEdgePoly().getMids();
            map = make_shared<Map>("mids map",poly);
        }
        Q_ASSERT(map);
        Q_ASSERT(!map->isEmpty());
        qDebug().noquote() << "     motif=" << map->summary() << "Tile Sides" << tile->numSides();
#else
        if (map)
            qDebug().noquote() << "     motif=" << map->summary() << "Tile Sides" << tile->numSides();
        else
            qDebug().noquote() << "     motif=" << "NO MAP" << "Tile Sides" << tile->numSides();
#endif
        adjacentTileMaps.insert(tile, map);
    }
    qDebug() << "motifMaps =" << adjacentTileMaps.size();

    // new map created after previous map has been copied imto adjacentTileMaps;
    motifMap = std::make_shared<Map>("Inferred Motif map");

    // Get the index of a good transform for this tile.
    int cur              = findPrimaryTile(tile);
    qDebug() << "primary feature index=" << cur;
    MidsPtr primaryMids  = allMotifMids[cur];
    Q_ASSERT(primaryMids);

    // adjacencies
    QVector<AdjacentTilePtr> adjacentTiles = getAdjacenctTiles(primaryMids, cur);
    qDebug() << "adjacenct tiles =" << adjacentTiles.size();

#ifdef DEBUG_CONTACTS
    for (auto & ai : adjacentTiles)
    {
        qDebug() << ai->placedTile->getTile()->toString();
    }
#endif

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

#ifdef DEBUG_CONTACTS
    debugContacts   = true;
    debugContactPts = contacts;

    for (auto & con : contacts)
    {
        qDebug().noquote() << con->toString();
    }
#endif

    QPolygonF priPts = primaryMids->getTransformedPoints();
    qDebug() << "Primary Points" << priPts.size();

    // For every contact, if it hasn't found an extension,
    // Look at all other contacts for likely candidates.
    for (const auto &contact : qAsConst(contacts))
    {
        if (contact->taken)
        {
            continue;
        }

        ContactPtr bestocon;
        QPointF    bestisect;
        qreal      bestdist = 0.0;
        eKind      bestkind = INFER_NONE;

        for (const auto &ocon : qAsConst(contacts))
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
                mydist = Point::dist(contact->position,ocon->position);
            }
            else
            {
                // We don't want the case where the intersection lies too close to either vertex.
                if (Intersect::getTrueIntersection( contact->position, contact->end, ocon->position, ocon->end, isect))
                {
                    qreal dist  = Point::dist(contact->position,isect );
                    qreal odist = Point::dist(ocon->position,isect );

                    bool inside = priPts.containsPoint(isect,Qt::OddEvenFill);

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
            ocon->colinear = Contact::COLINEAR_SLAVE;
        }
        else
        {
            if (debugContacts) qDebug() << "isect: " << bestisect << contact.get() << ocon.get();
            contact->isect  = bestisect;
            ocon->isect = bestisect;
        }

        contact->isect_contact = bestocon;
        ocon->isect_contact    = contact;
    }

#ifdef DEBUG_CONTACTS
    for (auto & con : contacts)
    {
        qDebug().noquote() << con->toString();
    }
#endif

    // Using the stored intersections in the contacts, build an inferred map.
    for (const auto & contact : contacts)
    {
        //contact->dump();
        if (contact->isect.isNull())
        {
            if (contact->colinear == Contact::COLINEAR_MASTER)
            {
                auto edge = motifMap->insertEdge(contact->position, contact->isect_contact.lock()->position );
                if (debugContacts) qDebug().noquote() << "Pass 1 inserting edge A:" << edge->dump() << motifMap->summary();
            }
        }
        else
        {
            auto edge = motifMap->insertEdge(contact->position, contact->isect );
            if (debugContacts) qDebug().noquote() << "Pass 1 inserting edge B:" << edge->dump() << motifMap->summary();
        }
    }

    // Try to link up unlinked edges.
    qreal minlen = Point::dist(priPts[0], priPts[priPts.size()-1] );
    for( int idx = 1; idx < priPts.size(); ++idx )
    {
        minlen = std::min( minlen, Point::dist(priPts[idx-1], priPts[ idx ] ) );
    }

#ifdef SUPPORT_NOT_PIC
    // DAC another pass where there is no isect because polygons are not in contact
    for (const auto &con : contacts)
    {
        if (!con->isect_contact.lock())
        {
            for (const auto &ocon : contacts )
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
                tmp = Point::normalize(tmp);
                tmp *= (minlen*0.5);
                QPointF ex1 = con->position + tmp;  // DAC hard to decipher the java precedence rules used here

                tmp  = ocon->position - ocon->other;
                tmp = Point::normalize(tmp);
                tmp *= (minlen*0.5);
                QPointF ex2 = ocon->position + tmp; // ditto

                auto edge = motifMap->insertEdge( con->position, ex1 );
                qDebug().noquote() << "inserting edge:" << edge->dump() << motifMap->summary();
                edge = motifMap->insertEdge( ex1, ex2 );
                qDebug().noquote() << "inserting edge:" << edge->dump() << motifMap->summary();
                edge = motifMap->insertEdge( ex2, ocon->position );
                qDebug().noquote() << "inserting edge:" << edge->dump() << motifMap->summary();

                con->isect_contact  = ocon;
                ocon->isect_contact = con;
            }
        }
    }
#endif

    motifMap->transformMap(primaryMids->getTransform().inverted());

    motifMap->verify();

    if (motifMap->isEmpty())
    {
        qWarning() << "Inferred Motif Map is emtpty";
    }

    if (debugContacts) qDebug() << "InferredMotif::infer() - END" << motifMap->namedSummary();
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
    for (const auto & adj : qAsConst(adjs))
    {
        MapPtr motifMap = adjacentTileMaps.value(adj->placedTile->getTile());
        if (motifMap)
        {
            if (debugContacts) qDebug().noquote() << "Existing adjacent map"  << motifMap->summary();
            MapPtr placedMotifMap = motifMap->recreate();
            placedMotifMap->transformMap(adj->placedTile->getTransform());
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

        for (const auto & v : qAsConst(map->getVertices()))
        {
            QPointF pos = v->pt;
            qreal dist2 = Point::dist2ToLine(pos, pt, nextPt);
            if (Loose::Near(dist2, adj->tolerance) &&  !Loose::Near(pos, pt, adj->tolerance) &&  !Loose::Near(pos, nextPt, adj->tolerance))
            {
                // This vertex lies on the edge.  Add all its edges to the contact list.
                NeighboursPtr n = map->getNeighbours(v);
                for (auto & wedge : qAsConst(*n))
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

int InferredMotif::lexCompareDistances( int kind1, qreal dist1, int kind2, qreal dist2 )
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

void InferredMotif::setupInfer(ProtoPtr proto)
{
    this->proto = proto;

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
            QPointF pt   = (tiling->getData().getTrans1() * static_cast<qreal>(x)) + (tiling->getData().getTrans2() * static_cast<qreal>(y));
            QTransform T = QTransform::fromTranslate(pt.x(),pt.y());

            const PlacedTiles & placedTiles = tiling->getInTiling();
            for (const auto & placedTile : placedTiles)
            {
                QTransform Tf   = placedTile->getTransform() * T;
                TilePtr tile    = placedTile->getTile();

                QPolygonF t_pts = Tf.map(tile->getPoints());
                EdgePoly t_ep   = tile->getEdgePoly().map(Tf);

                Points tileMidpoints;
                int sz = tile->numPoints();
                for(int idx = 0; idx < sz; ++idx )
                {
                    tileMidpoints << Point::convexSum(t_pts[idx], t_pts[(idx+1)%sz], 0.5 );
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
    int count = tiling->getInTiling().count();
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
    for (auto pt: mid_points)
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
            for (auto mid : mid_points)
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
