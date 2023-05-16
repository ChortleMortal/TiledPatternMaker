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

#include <QMessageBox>
#include <QDebug>

#include "geometry/map.h"
#include "misc/qtapplog.h"
#include "misc/timers.h"
#include "settings/configuration.h"
#include "widgets/dlg_textedit.h"
#include "panels/panel.h"
#include "geometry/neighbours.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"

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

bool Map::verify(bool force)
{
    auto errors = _verify(force);
    return (errors.size() == 0);
}

bool Map::verifyAndFix(bool force, bool confirm)
{
    auto errors = _verify(force);
    if (errors.size())
    {
        if (confirm)
        {
            if (errors.contains(MAP_EMPTY))
            {
                qWarning("Empty map");
                QMessageBox box(ControlPanel::getInstance());
                box.setIcon(QMessageBox::Warning);
                box.setText("XML Writer: Map is empty");
                box.exec();
                return true;
            }

            QMessageBox box(ControlPanel::getInstance());
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            box.setText("XML Writer: Map verify failed\n\nProceed to cleanse ?");
            int ret = box.exec();
            if (ret == QMessageBox::No)
            {
                return true;
            }
        }

        unsigned int options = 0;
        if (errors.contains(MAP_EMPTY))
        {
            qWarning("Empty map");
        }
        if (errors.contains(MERR_EDGES_INTERSECTING))
        {
            options |= divideupIntersectingEdges;
        }
        if (errors.contains(NEIGHBOUR_NO_EDGE))
        {
            options |= buildNeighbours;
        }
        if (errors.contains(EDGE_DUPLICATED))
        {
            options |= cleanupNeighbours;
        }
        if (errors.contains(EDGE_VERTEX_UNKNOWN))
        {
            options |= badEdges;
        }
        if (errors.contains(EDGE_TRIVIAL_VERTICES))
        {
            options |= badEdges;    // same as above
        }
        AQElapsedTimer timer;
        cleanse(options);
        qDebug(). noquote() << "cleanse :" << timer.getElapsed();

        errors = _verify(force);
        if (procErrors(errors))
        {
            QString msg = "Map verify failed after cleanse";
            qWarning().noquote() << msg;
            QMessageBox box(ControlPanel::getInstance());
            box.setIcon(QMessageBox::Warning);
            box.setText(msg);
            box.exec();
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

QVector<eMapError> Map::_verify(bool force)
{
    errors.clear();
    if (!config->verifyMaps && !force)
    {
        return errors;
    }

    qtAppLog * log = qtAppLog::getInstance();
    if (!config->dontTrapLog)
    {
        log->trap(true);   // traps a copy of debug info
    }
    qDebug().noquote() << "$$$$ Verifying:" << mname << "Vertices:" << vertices.size()  << "Edges:" << edges.size();

    if (vertices.size() == 0 && edges.size() == 0)
    {
        qDebug() << "empty map";
        errors.push_back(MAP_EMPTY);
        goto windup;
    }

    if (edges.size() == 0)
    {
        qWarning() << "No edges";
        errors.push_back(MAP_NO_EDGES);
        goto windup;
    }

    if (vertices.size() == 0)
    {
        qWarning() << "no vertices";
        errors.push_back(MAP_NO_VERTICES);
        goto windup;
    }

    if (!nMap)
    {
        qInfo( ) << "neighbours are not built";
        errors.push_back(NEIGHBOUR_NO_EDGE);
    }

    if (config->verifyDump)
    {
        dumpMap(true);
    }

    verifyEdges();
    //verifyVertices();
    verifyNeighbours();

    qDebug() << "$$$$ Verify end";

windup:
    if (procErrors(errors))
    {
        qWarning().noquote() << "Verify ERROR" << mname << "did NOT verify!"  <<  "Vertices:" << vertices.size() << "Edges:" << edges.size();

        if (config->verifyPopup)
        {
            DlgMapVerify dlg(mname,ControlPanel::getInstance());
            dlg.set(qtAppLog::getTrap());
            dlg.exec();
        }
    }
    else
    {
        qDebug().noquote() << mname << "Verify OK" <<  "Vertices:" << vertices.size() << "Edges:" << edges.size();
    }

    if (!config->dontTrapLog)
    {
        log->trapClear();
        log->trap(false);
    }

    return errors;
}

void Map::verifyEdges()
{
    for (auto edge: qAsConst(edges))
    {
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        if (edge->getType() != EDGETYPE_CURVE && edge->getType() != EDGETYPE_CHORD)
        {
            if (config->verifyVerbose) qDebug() <<  "verifying edge (" << edgeIndex(edge) << ") : from" << vertexIndex(v1) << "to"  << vertexIndex(v2);
        }
        else
        {
            if (config->verifyVerbose) qDebug() <<  "verifying CURVED edge (" << edgeIndex(edge) << ") : from" << vertexIndex(v1) << "to"  << vertexIndex(v2);
        }

        if (v1 == v2)
        {
            qWarning() << "Trivial edge " << edgeIndex(edge) << "v1==v2" << vertexIndex(v1) << vertexIndex(v2);
            errors.push_back(EDGE_TRIVIAL_VERTICES);
            continue;
        }

        if (v1->pt == v2->pt)
        {
            qWarning() << "edge" << edgeIndex(edge) << "not really an edge";
            errors.push_back(EDGE_TRIVIAL_POINTS);
            continue;
        }

        // Make sure that for every edge, the edge's endpoints
        // are know vertices in the map.
        if (!vertices.contains(v1))
        {
            qWarning() << "edge" << edgeIndex(edge) << "V1 not in vertex list.";
            errors.push_back(EDGE_VERTEX_UNKNOWN);
        }
        if (!vertices.contains(v2))
        {
            qWarning() << "edge" << edgeIndex(edge) << "V2 not in vertex list.";
            errors.push_back(EDGE_VERTEX_UNKNOWN);
        }

        if (nMap)
        {
            // make sure the edge is the only connection between the two endpoints
            //qDebug() << "   V1";
            auto neighbours = getNeighbours(v1);
            for (auto & wedge : *neighbours)
            {
                EdgePtr eother = wedge.lock();

                if (!eother)
                    continue;

                if (eother == edge)
                    continue;

                if (eother->sameAs(edge))
                {
                    qWarning() << "v1 sameAs() duplicate edges" << edgeIndex(edge) << edgeIndex(eother);
                    errors.push_back(EDGE_DUPLICATED);
                    break;
                }
                else if (eother->equals(edge))
                {
                    qWarning() << "v1 equals() duplicate edges" << edgeIndex(edge) << edgeIndex(eother);
                    errors.push_back(EDGE_DUPLICATED);
                    break;
                }
            }

            //qDebug() << "   V2";
            neighbours = getNeighbours(v2);
            for (auto & wedge : *neighbours)
            {
                EdgePtr eother = wedge.lock();

                if (!eother)
                    continue;

                if (eother == edge)
                    continue;

                if (eother->sameAs(edge))
                {
                    qWarning() << "v2 sameAs() duplicate edges" << edgeIndex(edge) <<edgeIndex(eother);
                    errors.push_back(EDGE_DUPLICATED);
                    break;
                }
                else if (eother->equals(edge))
                {
                    qWarning() << "v2 equals() duplicate edges" << edgeIndex(edge) <<edgeIndex(eother);
                    errors.push_back(EDGE_DUPLICATED);
                    break;
                }
            }
        }
    }
}

void Map::verifyNeighbours()
{
    if (!nMap)
    {
        return;
    }

    // Make sure the vertices each have a neighbour and all neighbours are good
    for (const auto & vertex : qAsConst(vertices))
    {
        NeighboursPtr neighbour = getNeighbours(vertex);
        if (!neighbour->verify())
        {
            errors.push_back(NEIGHBOUR_INVALID_EDGE);
            continue;
        }

        if (neighbour->numNeighbours() == 0)
        {
            qWarning() << "Vertex" << vertexIndex(vertex) << "at position" << vertex->pt << "has no neigbours, is floating in space";
            errors.push_back(NEIGHBOUR_NO_EDGE);
        }

        for (const auto & wedge : *neighbour)
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
                qWarning() << "vertex" << vertexIndex(vertex) << "not found in neighbouring edges";
                errors.push_back(NEIGHBOUR_BAD);
            }
            if (!edges.contains(edge))
            {
                qWarning() << "Unknown edge" <<edgeIndex(edge) <<  "in neigbours list";
                errors.push_back(NEIGHBOUR_BAD);
            }
        }
    }
}


bool Map::procErrors(const QVector<eMapError> & errors)
{
    bool severe = false;
    for (auto err : errors)
    {
        if (isSevereError(err))
        {
            qWarning().noquote() << "Error:" <<sMapErrors[err];
            severe = true;
        }
        else
        {
            qInfo().noquote() << "Ignoring:" <<sMapErrors[err];
        }
    }
    return severe;
}

bool Map::isMinorError(eMapError err)
{
    switch(err)
    {
    case NEIGHBOUR_NO_EDGE:
        return true;

    default:
        return false;
    }
}

bool Map::isSevereError(eMapError err)
{
    return !isMinorError(err);
}

void Map::dumpErrors(const QVector<eMapError> & theErrors)
{
    for (auto err : theErrors)
    {
        qWarning().noquote() << "Error" << sMapErrors[err];
    }
}
