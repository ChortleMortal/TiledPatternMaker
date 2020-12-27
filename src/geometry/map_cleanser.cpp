#include "geometry/map_cleanser.h"
#include "geometry/map.h"
#include "geometry/loose.h"
#include "base/configuration.h"
#include "base/misc.h"
#include "panels/dlg_textedit.h"

MapCleanser::MapCleanser(MapPtr map)
{
    this->map = map;
    config = Configuration::getInstance();

    deb = new QDebug(&astring);
}

MapCleanser::~MapCleanser()
{
}

// A big 'ole sanity check for maps.  Throw together a whole collection
// of consistency checks.  When debugging maps, call early and call often.
//
// It would probably be better to make this function provide the error
// messages through a return value or exception, but whatever.

bool MapCleanser::verifyMap(QString mapname, bool force)
{
    if (!config->verifyMaps && !force)
    {
        return true;
    }

    bool good = true;

    (*deb).noquote() << "$$$$ Verifying:" << mapname << "[" << map->mname << "]" << "Edges:" << map->edges.size() << "Vertices:" << map->vertices.size() << "Neighbours:" << map->neighbourMap.size() << "Neighbouring edges:" << map->neighbourMap.countNeighbouringEdges() << endl;

    if (map->vertices.size() == 0 && map->edges.size() == 0)
    {
        qDebug() << "empty map";
        return true;
    }

    if (map->edges.size() == 0)
    {
        *deb << "WARNING: No edges" << endl;
        good = false;
        goto windup;
    }

    if (map->vertices.size() == 0)
    {
        *deb << "WARNING: no vertices" << endl;
        good = false;
        goto windup;
    }

    if (map->neighbourMap.size() == 0)
    {
        *deb << "WARNING: No neigbours" << endl;
        good = false;
        goto windup;
    }

    if (config->verifyDump)
    {
        map->dumpMap(true);
    }
    else
    {
        setTmpIndices();
    }

    if (!verifyVertices())
        good = false;

    if (!verifyEdges())
        good = false;

    if (!verifyNeighbours())
        good = false;

    (*deb) << "$$$$ Verify end" << endl;

 windup:
    if (!good)
    {
        (*deb).noquote() << "ERROR" << mapname << "[" << map->mname << "]" << "did NOT verify!"  << "(Edges:" << map->edges.size() << "Vertices:" << map->vertices.size()
                         << "Neighbours:"  << map->neighbourMap.size() << "Neighbouring edges:" << map->neighbourMap.countNeighbouringEdges() << ")" << endl;
 #if 1
        DlgTextEdit dlg(false);
        dlg.set(astring);
        dlg.exec();
#else
        DlgTextEdit * dlg = new DlgTextEdit(true);
        dlg->set(astring);
        dlg->show();
#endif
    }
    else
    {
        qDebug().noquote() << mapname << "[" << map->mname << "]" << "verify OK" <<  "(Edges:" << map->edges.size() << "Vertices:" << map->vertices.size()
                           << "Neighbours:" << map->neighbourMap.size() << "Neighbouring edges:" << map->neighbourMap.countNeighbouringEdges() << ")";
    }

    return good;
}

void MapCleanser::cleanse()
{


    const bool debug = false;

    qDebug() << "Map::cleanse - start";

    verifyMap("cleanse-start");

    removeBadEdges();
    if (debug) verifyMap("Map:removeBadEdges");

    fixNeighbours();
    if (debug) verifyMap("Map:fixNeighbours");

    removeDanglingVertices();
    if (debug) verifyMap("Map:removeDanglingVertices");

    removeBadEdgesFromNeighboursMap();
    if (debug) verifyMap("Map:removeBadEdgesFromNeighboursMap");

#if 0
    divideIntersectingEdges();
    if (debug) verifyMap("Map:divideIntersectingEdges");
#endif

#if 0
    joinColinearEdges();
    if (debug) verifyMap("Map:joinColinearEdges");
#endif

    cleanNeighbours();
    if (debug) verifyMap("Map:cleanNeighbours");

    map->neighbourMap.cleanse();
    if (debug) verifyMap("NeighbourMap::cleanse");

    map->sortAllNeighboursByAngle();
    if (debug) verifyMap("Map:sortAllNeighboursByAngle");

    map->sortVertices();
    if (debug) verifyMap("Map:sortVertices");

    map->sortEdges();

    verifyMap("cleanse-end");
    qDebug() << "Map::cleanse - end";
}

bool MapCleanser::verifyVertices()
{
    bool rv = true;

    for (auto v : qAsConst(map->vertices))
    {
        NeighboursPtr np = map->neighbourMap.getNeighbours(v);
        if (!map->neighbourMap.contains(v))
        {
            *deb << "WARNING: vertex" << v->getTmpVertexIndex() << "not found in neighbour map" << endl;
            rv = false;
        }
        else if (np->numNeighbours() == 0)
        {
            *deb << "WARNING: vertex" << v->getTmpVertexIndex() << "is disconnected - has no neighbours" << endl;
            rv = false;
        }
    }

    // Make sure the vertices are in sorted order.
    for( int idx = 1; idx < map->vertices.size(); ++idx )
    {
        VertexPtr v1 = map->vertices.at( idx - 1 );
        VertexPtr v2 = map->vertices.at( idx );

        int cmp = map->lexComparePoints( v1->getPosition(), v2->getPosition() );

        if( cmp == 0 )
        {
            *deb << "WARNING: Duplicate vertices:" << v1->getTmpVertexIndex() << v1->getPosition() <<  v2->getTmpVertexIndex() << v2->getPosition() << endl;
            rv = false;
        }
        else if( cmp > 0 )
        {
            *deb << "WARNING: Sortedness check failed for map->vertices." << endl;
            rv = false;
        }
    }

    return rv;
}

bool MapCleanser::verifyEdges()
{
    bool rv = true;

    for (auto edge: qAsConst(map->edges))
    {
        VertexPtr v1 = edge->getV1();
        VertexPtr v2 = edge->getV2();

        if (v1 == v2)
        {
            *deb << "WARNING Trivial edge " << edge->getTmpEdgeIndex() << "v1==v2" << v1->getTmpVertexIndex() << endl;
            rv = false;
        }

        if (edge->getType() != EDGETYPE_CURVE)
        {
            if (config->verifyVerbose) *deb <<  "verifying edge (" << edge->getTmpEdgeIndex() << ") : from" << v1->getTmpVertexIndex() << "to"  << v2->getTmpVertexIndex() << endl;
        }
        else
        {
            if (config->verifyVerbose) *deb <<  "verifying CURVED edge (" << edge->getTmpEdgeIndex() << ") : from" << v1->getTmpVertexIndex() << "to"  << v2->getTmpVertexIndex() << endl;
        }

        if (v1->getPosition() == v2->getPosition())
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "not really an edge" << endl;
            rv = false;
        }

        // Make sure that for every edge, the edge's endpoints
        // are know vertices in the map.
        if (!map->vertices.contains(v1))
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "V1 not in vertex list." << endl;
            rv = false;
        }
        if (!map->vertices.contains(v2))
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "V2 not in vertex list." << endl;
            rv = false;
        }

        // Make sure that the edge's endpoints know about the
        // edge, and that the edge is the only connection between
        // the two endpoints.
        //qDebug() << "   V1";
        NeighboursPtr np = map->neighbourMap.getNeighbours(v1);
        EdgePtr ed       = np->getNeighbour(v2);
        if (!ed)
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "not found in vertex v1 neighbours for vertex" <<  v1->getTmpVertexIndex() << endl;
            rv = false;
        }
        else if (ed != edge )
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "not matched in vertex v1 neighbours for vertex" << v1->getTmpVertexIndex() << endl;
            if (ed->sameAs(edge))
            {
                *deb << "WARNING: woops:  edge same as other edge" << endl;
            }
            rv = false;
        }

        //qDebug() << "   V2";
        np = map->neighbourMap.getNeighbours(v2);
        ed = np->getNeighbour(v1);
        if (!ed)
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "not found in vertex v2 neighbours for vertex" <<  v2->getTmpVertexIndex() << endl;
            rv = false;
        }
        else if (ed != edge)
        {
            *deb << "WARNING: edge" << edge->getTmpEdgeIndex() << "not found in vertex v2 neighbours for vertex" <<  v2->getTmpVertexIndex() << endl;
            rv = false;
        }
    }

    // Make sure the edges are in sorted order.
    for( int idx = 1; idx < map->edges.size(); ++idx )
    {
        EdgePtr e1 = map->edges.at( idx - 1 );
        EdgePtr e2 = map->edges.at( idx );

        double e1x = e1->getMinX();
        double e2x = e2->getMinX();

        if( e1x > (e2x + Loose::TOL) )
        {
            *deb << "INFO: Sortedness check failed for map->edges." << endl;
        }
    }
    return rv;
}

bool MapCleanser::verifyNeighbours()
{
    bool rv = true;

    if (!map->neighbourMap.verify(deb))
    {
        rv = false;
    }

    // Make sure the vertices each have a neighbour and all neigours are good
    for (auto vp : qAsConst(map->vertices))
    {
        NeighboursPtr np = map->neighbourMap.getNeighbours(vp);
        if (np->numNeighbours() == 0)
        {
            *deb << "WARNING: Vertex" << vp->getTmpVertexIndex() << "at position" << vp->getPosition() << "has no neigbours, is floating in space" << endl;
            rv = false;
        }
        for (auto edge : qAsConst(np->getNeighbours()))
        {
            if (!edge)
            {
                *deb << "WARNING: Bad neighbour: no edge" << endl;
                rv = false;
            }
            else if (!edge->getV1())
            {
                *deb << "WARNING: Bad neighbour edge: no V1" << endl;
                rv = false;
            }
            else if (!edge->getV2())
            {
                *deb << "WARNING: Bad neighbour edge: no V2" << endl;
                rv = false;
            }
            if (!map->edges.contains(edge))
            {
                *deb << "WARNING: Unknown edge" << edge->getTmpEdgeIndex() <<  "in neigbours list" << endl;
                rv = false;
            }
        }
    }
    return rv;
}

void MapCleanser::setTmpIndices() const
{
    int idx = 0;
    for (auto v : map->vertices)
    {
        v->setTmpVertexIndex(idx++);
    }

    idx = 0;
    for (auto edge : map->edges)
    {
        edge->setTmpEdgeIndex(idx++);
    }
}



void MapCleanser::removeDanglingVertices()
{
    qDebug() << "removeDanglingVertices";

    QVector<VertexPtr> baddies;

    for (auto & v : map->vertices)
    {
        // examining a vertex
        NeighboursPtr np = map->neighbourMap.getNeighbours(v);
        if (np->numNeighbours() == 0)
        {
            baddies.push_back(v);
        }
    }

    for (auto& v : baddies)
    {
        map->removeVertex(v);
    }
}

void MapCleanser::removeBadEdges()
{
    qDebug() << "removeBadEdges";

    QVector<EdgePtr> baddies;

    for (auto e : qAsConst(map->edges))
    {
        // examining a vertex
        if (e->getV1() == e->getV2())
        {
            qDebug() << "removing null edge (1)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
        else if (e->getV1()->getPosition() == e->getV2()->getPosition())
        {
            qDebug() << "removing null edge (2)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
        if (!e->getV1())
        {
            qDebug() << "not really an edge (3)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
        if (!e->getV2())
        {
            qDebug() << "not really an edge (4)" << e->getTmpEdgeIndex();
            baddies.push_back(e);
        }
    }

    for (auto & e : baddies)
    {
        map->removeEdge(e);
    }
}

void MapCleanser::removeBadEdgesFromNeighboursMap()
{
    qDebug() << "removeBadEdgesFromNeighboursMap";

    for (auto & vp : map->vertices)
    {
        NeighboursPtr np = map->neighbourMap.getNeighbours(vp);
        QVector<EdgePtr> baddies;
        for (auto& edge : np->getNeighbours())
        {
            if (!map->edges.contains(edge))
            {
                baddies.push_back(edge);
            }
        }
        for (auto& edge : baddies)
        {
            np->removeEdge(edge);
        }
    }
}

void MapCleanser::divideIntersectingEdges()
{
    qDebug() << "divideIntersectingEdges";

    UniqueQVector<QPointF> intersects;
    for(auto edge : map->edges)
    {
        // To check all intersections of this edge with edges in
        // the current map, we can use the optimization that
        // edges are stored in sorted order by min x.  So at the
        // very least, skip the suffix of edges whose minimum x values
        // are past the max x of the edge to check.
        // Casper 12DEC02 - removed optimisation and simplified code

        QLineF e = edge->getLine();
        for (auto cur : map->edges)
        {
            if (cur == edge)
            {
                continue;
            }

            QLineF  c  = cur->getLine();
            QPointF apt;
            if (e.intersects(c,&apt) == QLineF::BoundedIntersection)
            {
                intersects.push_back(apt);
            }
        }
    }
    // now split the edges
    for (QPointF pt : intersects)
    {
        //qDebug() << "New split at" << pt;
        map->getVertex_Complex(pt);
    }

#if 0
    sortVertices();
    sortEdges();
    cleanNeighbours();
#endif
}

void MapCleanser::joinColinearEdges()
{
    qDebug() << "joinColinearEdges";

    bool changed = false;
    do
    {
        changed = joinOneColinearEdge();
        //verifyMap("joinColinearEdges",false,true);
    } while (changed);

#if 0
    sortVertices();
    sortEdges();
    cleanNeighbours();
    setTmpIndices();
#endif
}

bool MapCleanser::joinOneColinearEdge()
{
    for (auto& vp : map->vertices)
    {
        int count = map->neighbourMap.numNeighbours(vp);
        if (count == 2)
        {
            NeighboursPtr np = map->neighbourMap.getNeighbours(vp);
            QLineF a = np->getEdge(0)->getLine();
            QLineF b = np->getEdge(1)->getLine();
            qreal angle = a.angle(b);   // FIXME deprecated
            if (Loose::zero(angle) || Loose::equals(angle,180.0))
            {
                // need to remove one edge, extend the other, and remove vertex
                combineLinearEdges(np->getEdge(0),np->getEdge(1),vp);
                return true;
            }
        }
        else if (count == 1)
        {
#if 0
            // this is a dangling edge which sometimes is ok
            EdgePtr      ep = vp->getNeighbours().at(0);
            QLineF        a = ep->getLine();
            VertexPtr other = ep->getOther(vp->getPosition());
            QVector<EdgePtr> qvep = other->getEdges();
            for (auto it = qvep.begin(); it != qvep.end(); it++)
            {
                EdgePtr otherEdge = *it;
                QLineF          b = otherEdge->getLine();
                if (a == b)
                {
                    continue;
                }
                qreal       angle = a.angle(b);     // FIXME - deprecated
                if (Loose::zero(angle) || Loose::equals(angle,180.0))
                {
                    // this is a case where it needs to be deleeted
                    removeVertex(vp);
                    return true;
                }
            }
#endif
        }
    }
    return false;
}

// combine two edges which are in a straight line with common vertex
void MapCleanser::combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common)
{
    VertexPtr newV1 = a->getOtherV(common);
    VertexPtr newV2 = b->getOtherV(common);

    map->insertEdge(newV1,newV2);
    map->removeEdge(a);
    map->removeEdge(b);

#if 0
    VertexPtr av1 = a->getV1();
    VertexPtr av2 = a->getV2();
    VertexPtr bv1 = b->getV1();
    VertexPtr bv2 = b->getV2();
    VertexPtr modifiedVp;

    // let's keep a, and remove b
    if (av1 == common)
    {
        if (bv1 == common)
        {
            modifiedVp = bv2;
        }
        else
        {
            Q_ASSERT(bv2 == common);
            modifiedVp = bv1;
        }
        a->setV1(modifiedVp);
    }
    else
    {
        Q_ASSERT(av2 == common);
        if (bv1 == common)
        {
            modifiedVp = bv2;
        }
        else
        {
            Q_ASSERT(bv2 == common);
            modifiedVp = bv1;
        }
        a->setV2(modifiedVp);
    }

    // need to change the neighbours for modified vertex
    removeEdge(b);
    //neighbourMap.replaceNeighbour(modifiedVp,b,a);
#endif
}

void MapCleanser::cleanNeighbours()
{
    qDebug() << "cleanNeighbours BEGIN edges=" << map->edges.size()  << "vertices=" << map->vertices.size();

    for (auto & v :  map->vertices)
    {
        // examining a vertex
        NeighboursPtr np =map-> neighbourMap.getNeighbours(v);
        QVector<EdgePtr> & edgeVec = np->getNeighbours();
        deDuplicateEdges(edgeVec);
    }
    qDebug() << "cleanNeighbours END   edges=" << map->edges.size()  << "vertices=" << map->vertices.size();
}

void MapCleanser::fixNeighbours()
{
    qDebug() << "fixNeighbours";

    int index = 200;
    for (auto e : qAsConst(map->edges))
    {
        VertexPtr v1 = e->getV1();
        if (!map->vertices.contains(v1))
        {
            map->vertices.push_back(v1);
            v1->setTmpVertexIndex(index++);
        }

        VertexPtr v2 = e->getV2();
        if (!map->vertices.contains(v2))
        {
            map->vertices.push_back(v1);
            v2->setTmpVertexIndex(index++);
        }

        NeighboursPtr np = map->neighbourMap.getNeighbours(v1);
        if (!np)
        {
            map->neighbourMap.insertNeighbour(v1,e);
        }
        else
        {
            if (!np->contains(e))
            {
                np->insertEdge(e);
            }
        }

        np = map->neighbourMap.getNeighbours(v2);
        if (!np)
        {
            map->neighbourMap.insertNeighbour(v2,e);
        }
        else
        {
            if (!np->contains(e))
            {
                np->insertEdge(e);
            }
        }
    }
}

void MapCleanser::deDuplicateVertices(QVector<VertexPtr> & vec)
{
    UniqueQVector<VertexPtr> avec;
    for (auto vp : vec)
    {
        avec.push_back(vp);
    }
    vec = avec;
}

void MapCleanser::deDuplicateEdges(QVector<EdgePtr> & vec)
{
    // simple removal of duplicate edge pointers
    UniqueQVector<EdgePtr> avec;
    for (auto ep : vec)
    {
        avec.push_back(ep);
    }
    vec = avec;

    // examining the positions edges of associated with the vertex
    QVector<EdgePtr> duplicateEdges;
    for (auto it = vec.begin(); it != vec.end(); it++)
    {
        EdgePtr e = *it;
        auto it2 = it;
        it2++;
        for ( ; it2 != vec.end(); it2++)
        {
            EdgePtr e2 = *it2;
            if (e->sameAs(e2))
            {
                duplicateEdges.push_back(e2);
            }
        }
    }

    // remove duplicates from the map
    for (auto e : duplicateEdges)
    {
        map->removeEdge(e);
    }
}

void MapCleanser::removeVerticesWithEdgeCount(int edgeCount)
{
    QVector<VertexPtr> verts;
    for (auto& v : map->vertices)
    {
        NeighboursPtr np = map->neighbourMap.getNeighbours(v);

        if (edgeCount == np->numEdges())
        {
            verts.push_back(v);
        }
    }

    for (auto& v : verts)
    {
        map->removeVertex(v);
    }
}
