////////////////////////////////////////////////////////////////////////////
//
// The implementation of a planar map abstraction.  A planar map is
// an (undirected) graph represented on the plane in such a way that
// edges don't cross vertices or other edges.
//
// This is one of the big daddy structures of computational geometry.
// The right way to do it is with a doubly-connected edge list structure,
// complete with half edges and a face abstraction.  Because I'm lazy
// and because of the sheer coding involved, I'm doing something simpler,
// more like an ordinary graph.  The disadvantage is that I don't maintain
// faces explicitly, which can make face colouring for islamic patterns
// tricky later.  But it's more tractable than computing overlays of
// DCELs.

#include <QDebug>
#include <QSet>

#include "sys/geometry/map_verifier.h"
#include "sys/qt/qtapplog.h"
#include "model/settings/configuration.h"
#include "model/makers/mosaic_maker.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/map_cleanser.h"

#define E2STR(x) #x

const QString sMapErrors[] =
{
    E2STR(MAP_EMPTY),
    E2STR(MAP_NO_EDGES),
    E2STR(MAP_NO_VERTICES),

    E2STR(EDGE_TRIVIAL_VERTICES),
    E2STR(EDGE_TRIVIAL_POINTS),
    E2STR(EDGE_VERTEX_UNKNOWN),
    E2STR(EDGE_DUPLICATED),

    E2STR(VERTEX_DUPLICATED),

    E2STR(NEIGHBOUR_NO_EDGE),
    E2STR(NEIGHBOUR_INVALID_EDGE),
    E2STR(NEIGHBOUR_EXPIRED),
    E2STR(NEIGHBOUR_BAD),

    E2STR(MERR_EDGES_INTERSECTING)
};

bool MapVerifier::verify(bool force)
{
    if (!Sys::config->verifyMaps && !force)
    {
        return true;
    }

    QVector<eMapError> errors = _verify();
    return (errors.size() == 0);
}

bool MapVerifier::verifyAndFix(bool force)
{
    if (!Sys::config->verifyMaps && !force)
    {
        return true;
    }

    QVector<eMapError> errors = _verify();
    if (errors.size())
    {
        uint options = 0;
        if (errors.contains(MAP_EMPTY))
        {
            qWarning("Empty map");
        }
        if (errors.contains(MERR_EDGES_INTERSECTING))
        {
            options |= divideupIntersectingEdges;
        }
        if (errors.contains(EDGE_DUPLICATED))
        {
            options |= cleanupEdges;
        }
        if (errors.contains(EDGE_VERTEX_UNKNOWN))
        {
            options |= coalesceEdges;
        }
        if (errors.contains(EDGE_TRIVIAL_VERTICES))
        {
            options |= coalesceEdges;    // same as above
        }

        if (options == 0)
        {
            return true;
        }

        // cleanse
        MapCleanser mc(map);
        mc.cleanse(options,Sys::config->mapedMergeSensitivity);

        // revrify
        errors = _verify();
        if (procErrors(errors))
        {
            QString msg = "Map verify failed after cleanse";
            qWarning().noquote() << msg;
            if (Sys::config->verifyPopup && Sys::isGuiThread())
            {
                emit Sys::mosaicMaker->sig_messageBox(msg,QString(),QMessageBox::Warning,false);
            }
            return false;
        }
        else
        {
            qDebug() << "Map verify after cleans: fixed OK";
        }

    }
    return (errors.size() == 0);
}

// A big 'ole sanity check for maps.  Throw together a whole collection
// of consistency checks.  When debugging maps, call early and call often.
//
// It would probably be better to make this function provide the error
// messages through a return value or exception, but whatever.

QVector<eMapError> MapVerifier::_verify()
{
    NeighbourMap * nmap = nullptr;

    qtAppLog * log = qtAppLog::getInstance();
    if (!Sys::dontTrapLog)
    {
        log->trap(true);   // traps a copy of debug info
    }
    qDebug().noquote() << "$$$$ Verifying:" << map->mname << "Vertices:" << map->vertices.size()  << "Edges:" << map->edges.size();

    if (map->vertices.size() == 0 && map->edges.size() == 0)
    {
        qDebug() << "Empty map";
        goto windup;
    }

    if (map->edges.size() == 0)
    {
        qWarning() << "No edges";
        goto windup;
    }

    if (map->vertices.size() == 0)
    {
        qWarning() << "no vertices";
        goto windup;
    }

    if (Sys::config->buildEmptyNmaps)
    {
        qInfo( ) << "Building Neighbour Map";
        nmap = new NeighbourMap(map);
    }

    if (Sys::config->verifyDump)
    {
        map->dump(true);
    }

    verifyEdges();

    verifyNeighbours(nmap);

    qDebug() << "$$$$ Verify end";

windup:
    if (procErrors(errors))
    {
        qWarning().noquote() << "Verify ERROR" << map->mname << "did NOT verify!"  <<  "Vertices:" << map->vertices.size() << "Edges:" << map->edges.size();

        if (Sys::config->verifyPopup && Sys::isGuiThread())
        {
            QStringList lines;
            for (const sTrapMsg & stm : qtAppLog::getTrap())
            {
                lines << stm.msg;
            }
            emit Sys::mosaicMaker->sig_messageBox(QString("Map verify problem with: %1").arg(map->mname),lines.join("\n"),QMessageBox::Warning,false);
        }
    }
    else
    {
        qInfo().noquote() << "Verify OK" << map->mname << "Vertices:" << map->vertices.size() << "Edges:" << map->edges.size();
    }

    if (!Sys::dontTrapLog)
    {
        log->trap(false);
    }

    if (nmap)
        delete nmap;

    return errors;
}

void MapVerifier::verifyEdges()
{
    // Built once and reused for every edge below - this is a pure read-only
    // check (verify never mutates the map), so a single upfront build is
    // always valid. Previously a fresh NeighbourMap (itself an O(E) scan)
    // was constructed inside this loop on every iteration, making the whole
    // function O(E^2) - the dominant cost on any map with more than a few
    // hundred edges.
    NeighbourMap nmap(map);

    QSet<Vertex*> knownVertices;
    for (const auto & v : std::as_const(map->vertices))
        knownVertices.insert(v.get());

    for (const auto & edge : std::as_const(map->edges))
    {
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        if (Sys::config->verifyVerbose)
        {
            if (edge->getType() != EDGETYPE_CURVE)
            {
                qDebug() <<  "verifying edge (" << map->edgeIndex(edge) << ") : from" << map->vertexIndex(v1) << "to"  << map->vertexIndex(v2);
            }
            else
            {
                qDebug() <<  "verifying CURVED edge (" << map->edgeIndex(edge) << ") : from" << map->vertexIndex(v1) << "to"  << map->vertexIndex(v2);
            }
        }

        if (v1 == v2)
        {
            qWarning() << "Trivial edge " << map->edgeIndex(edge) << "v1==v2" << map->vertexIndex(v1) << map->vertexIndex(v2);
            errors.push_back(EDGE_TRIVIAL_VERTICES);
            continue;
        }

        if (v1->pt == v2->pt)
        {
            qWarning() << "edge" << map->edgeIndex(edge) << "not really an edge";
            errors.push_back(EDGE_TRIVIAL_POINTS);
            continue;
        }

        // Make sure that for every edge, the edge's endpoints
        // are know vertices in the map.
        if (!knownVertices.contains(v1.get()))
        {
            qWarning() << "edge" << map->edgeIndex(edge) << "V1 not in vertex list.";
            errors.push_back(EDGE_VERTEX_UNKNOWN);
        }
        if (!knownVertices.contains(v2.get()))
        {
            qWarning() << "edge" << map->edgeIndex(edge) << "V2 not in vertex list.";
            errors.push_back(EDGE_VERTEX_UNKNOWN);
        }

        // make sure the edge is the only connection between the two endpoints
        //qDebug() << "   V1";
        auto neighbours = nmap.getNeighbours(v1);
        for (auto & wedge : std::as_const(*neighbours))
        {
            EdgePtr eother = wedge.lock();

            if (!eother)
                continue;

            if (eother == edge)
                continue;

            if (eother->sameAs(edge))
            {
                qWarning() << "v1 sameAs() duplicate edges" << map->edgeIndex(edge) << map->edgeIndex(eother);
                errors.push_back(EDGE_DUPLICATED);
                break;
            }
            else if (eother->equals(edge))
            {
                qWarning() << "v1 equals() duplicate edges" << map->edgeIndex(edge) << map->edgeIndex(eother);
                errors.push_back(EDGE_DUPLICATED);
                break;
            }
        }

        //qDebug() << "   V2";
        neighbours = nmap.getNeighbours(v2);
        for (auto & wedge : std::as_const(*neighbours))
        {
            EdgePtr eother = wedge.lock();

            if (!eother)
                continue;

            if (eother == edge)
                continue;

            if (eother->sameAs(edge))
            {
                qWarning() << "v2 sameAs() duplicate edges" << map->edgeIndex(edge) << map->edgeIndex(eother);
                errors.push_back(EDGE_DUPLICATED);
                break;
            }
            else if (eother->equals(edge))
            {
                qWarning() << "v2 equals() duplicate edges" << map->edgeIndex(edge) << map->edgeIndex(eother);
                errors.push_back(EDGE_DUPLICATED);
                break;
            }
        }
    }
}

void MapVerifier::verifyNeighbours(NeighbourMap * nMap)
{
    if (!nMap)
    {
        return;
    }

    // Built once, checked O(1) per neighbour below instead of the O(E) scan
    // map->edges.contains(edge) would do on every one of the E*avg_degree
    // neighbour visits this function makes.
    QSet<Edge*> knownEdges;
    for (const auto & e : std::as_const(map->edges))
        knownEdges.insert(e.get());

    // Make sure the vertices each have a neighbour and all neighbours are good
    for (const auto & vertex : std::as_const(map->vertices))
    {
        NeighboursPtr neighbour = nMap->getNeighbours(vertex);
        if (!neighbour->verify())
        {
            errors.push_back(NEIGHBOUR_INVALID_EDGE);
            continue;
        }

        if (neighbour->numNeighbours() == 0)
        {
            qWarning() << "Vertex" << map->vertexIndex(vertex) << "at position" << vertex->pt << "has no neigbours, is floating in space";
            errors.push_back(NEIGHBOUR_NO_EDGE);
        }

        for (const auto & wedge : std::as_const(*neighbour))
        {
            EdgePtr edge = wedge.lock();
            if (!edge)
            {
                qWarning() << "Bad neighbour: no edge";
                errors.push_back(NEIGHBOUR_EXPIRED);
            }
            else if (!edge->v1)
            {
                qWarning() << "Bad neighbour edge: no V1";
                errors.push_back(NEIGHBOUR_BAD);
            }
            else if (!edge->v2)
            {
                qWarning() << "Bad neighbour edge: no V2";
                errors.push_back(NEIGHBOUR_BAD);
            }
            else if (!edge->contains(vertex))
            {
                qWarning() << "vertex" << map->vertexIndex(vertex) << "not found in neighbouring edges";
                errors.push_back(NEIGHBOUR_BAD);
            }
            if (!knownEdges.contains(edge.get()))
            {
                qWarning() << "Unknown edge" << map->edgeIndex(edge) <<  "in neigbours list";
                errors.push_back(NEIGHBOUR_BAD);
            }
        }
    }
}


bool MapVerifier::procErrors(const QVector<eMapError> & errors)
{
    for (auto err : std::as_const(errors))
    {
        qWarning().noquote() << "Error:" <<sMapErrors[err];
    }
    return (errors.count() > 0);
}
