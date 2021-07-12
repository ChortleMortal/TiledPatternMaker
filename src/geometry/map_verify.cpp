/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include "geometry/map.h"
#include "settings/configuration.h"
#include "geometry/loose.h"
#include "panels/dlg_textedit.h"
#include "panels/panel.h"
#include "geometry/neighbours.h"
#include "geometry/vertex.h"
#include "geometry/edge.h"


// A big 'ole sanity check for maps.  Throw together a whole collection
// of consistency checks.  When debugging maps, call early and call often.
//
// It would probably be better to make this function provide the error
// messages through a return value or exception, but whatever.

bool Map::verify(bool force)
{
    if (!config->verifyMaps && !force)
    {
        return true;
    }

    bool good = true;

    qtAppLog::trap(true);
    qDebug().noquote() << "$$$$ Verifying:" << mname  << "Edges:" << edges.size() << "Vertices:" << vertices.size();

    if (vertices.size() == 0 && edges.size() == 0)
    {
        qDebug() << "empty map";
        return true;
    }

    if (edges.size() == 0)
    {
        qWarning() << "No edges";
        good = false;
        goto windup;
    }

    if (vertices.size() == 0)
    {
        qWarning() << "no vertices";
        good = false;
        goto windup;
    }

    if (!status.neighboursBuilt)
    {
        qInfo( )<< "neighbours are not built";
    }

    if (config->verifyDump)
    {
        dumpMap(true);
    }

    if (!verifyVertices())
        good = false;

    if (!verifyEdges())
        good = false;

    if (!verifyNeighbours())
        good = false;

    qDebug() << "$$$$ Verify end";

windup:
    if (!good)
    {
        qWarning().noquote() << "Verify ERROR" << mname << "did NOT verify!"  << "(Edges:" << edges.size() << "Vertices:" << vertices.size() << ")";

        if (!qtAppLog::trapEnabled())
        {
            DlgMapVerify dlg(mname,ControlPanel::getInstance());
            dlg.set(qtAppLog::getTrap());
            dlg.exec();
        }
        else
        {
            qtAppLog::forceTrapOutput();    // logs0 but does not pop up
        }
    }
    else
    {
        qDebug().noquote() << mname << "Verify OK" <<  "(Edges:" << edges.size() << "Vertices:" << vertices.size() << ")";
    }

    qtAppLog::trap(false);

    return good;
}

bool Map::verifyVertices()
{
    bool rv = true;

    if (status.neighboursBuilt)
    {
        for (const auto & v : qAsConst(vertices))
        {
            NeighboursPtr n = getRawNeighbours(v);

            if (n->numNeighbours() == 0)
            {
                qWarning() << "vertex" << vertexIndex(v) << "is disconnected - has no neighbours";
                rv = false;
            }
            else
            {
                std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
                for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
                {
                    WeakEdgePtr e = *pos;
                    if (e.expired())
                    {
                        qWarning() << "vertex" << vertexIndex(v) << " has expired edge in neighbours";
                        rv = false;
                    }
                    EdgePtr edge = e.lock();
                    if (!edge->contains(v))
                    {
                        qWarning() << "vertex" << vertexIndex(v) << "not found in neighbouring edges";
                        rv = false;
                    }
                }
            }
        }
    }

    // Make sure the vertices are in sorted order.
    for( int idx = 1; idx < vertices.size(); ++idx )
    {
        VertexPtr v1 = vertices.at( idx - 1 );
        VertexPtr v2 = vertices.at( idx );

        int cmp = lexComparePoints( v1->pt, v2->pt );

        if( cmp == 0 )
        {
            qWarning() << "Duplicate vertices:" << vertexIndex(v1) << v1->pt <<  vertexIndex(v2) << v2->pt;
            rv = false;
        }
        else if( cmp > 0 )
        {
            qWarning() << "WARNING: Sortedness check failed for vertices.";
            rv = false;
            break;
        }
    }

    return rv;
}

bool Map::verifyEdges()
{
    bool rv = true;

    for (auto edge: qAsConst(edges))
    {
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        if (edge->getType() != EDGETYPE_CURVE)
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
            rv = false;
            continue;
        }

        if (v1->pt == v2->pt)
        {
            qWarning() << "edge" << edgeIndex(edge) << "not really an edge";
            rv = false;
        }

        // Make sure that for every edge, the edge's endpoints
        // are know vertices in the map.
        if (!vertices.contains(v1))
        {
            qWarning() << "edge" << edgeIndex(edge) << "V1 not in vertex list.";
            rv = false;
        }
        if (!vertices.contains(v2))
        {
            qWarning() << "edge" << edgeIndex(edge) << "V2 not in vertex list.";
            rv = false;
        }

        if (status.neighboursBuilt)
        {
            // make sure the edge is the only connection between the two endpoints
            //qDebug() << "   V1";
            NeighboursPtr n = getRawNeighbours(v1);
            std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
            for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
            {
                WeakEdgePtr wedge = *pos;
                EdgePtr eother = wedge.lock();
                if (eother == edge)
                    continue;

                if (eother->sameAs(edge))
                {
                    qWarning() << "v1 sameAs() duplicate edges" <<edgeIndex(edge) <<edgeIndex(eother);
                    rv =false;
                }
                else if (eother->equals(edge))
                {
                    qWarning() << "v1 equals() duplicate edges" <<edgeIndex(edge) <<edgeIndex(eother);
                    rv =false;
                }
            }

            //qDebug() << "   V2";
            n = getRawNeighbours(v2);
            std::vector<WeakEdgePtr> * wedges2 = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
            for (auto pos = wedges2->begin(); pos != wedges2->end(); pos++)
            {
                WeakEdgePtr wedge = *pos;
                EdgePtr eother = wedge.lock();
                if (eother == edge)
                    continue;

                if (eother->sameAs(edge))
                {
                    qWarning() << "v2 sameAs() duplicate edges" << edgeIndex(edge) <<edgeIndex(eother);
                    rv =false;
                }
                else if (eother->equals(edge))
                {
                    qWarning() << "v2 equals() duplicate edges" << edgeIndex(edge) <<edgeIndex(eother);
                    rv =false;
                }
            }
        }
    }

    // Make sure the edges are in sorted order.
    for( int idx = 1; idx <edges.size(); ++idx )
    {
        EdgePtr e1 =edges.at( idx - 1 );
        EdgePtr e2 =edges.at( idx );

        double e1x = e1->getMinX();
        double e2x = e2->getMinX();

        if( e1x > (e2x + Loose::TOL) )
        {
            qInfo() << "Sortedness check failed for edges.";
            rv = false;
            break;
        }
    }
    return rv;
}

bool Map::verifyNeighbours()
{
    if (!status.neighboursBuilt)
    {
        return true;
    }

    bool rv = true;

    // Make sure the vertices each have a neighbour and all neighbours are good
    for (const auto & vp : qAsConst(vertices))
    {
        NeighboursPtr n = getRawNeighbours(vp);

        rv = n->verify();

        if (n->numNeighbours() == 0)
        {
            qWarning() << "Vertex" <<vertexIndex(vp) << "at position" << vp->pt << "has no neigbours, is floating in space";
            rv = false;
        }
        std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
        for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
        {
            WeakEdgePtr wedge = *pos;
            EdgePtr edge = wedge.lock();
            if (!edge)
            {
                qWarning() << "Bad neighbour: no edge";
                rv = false;
            }
            else if (!edge->v1)
            {
                qWarning() << "Bad neighbour edge: no V1";
                rv = false;
            }
            else if (!edge->v2)
            {
                qWarning() << "Bad neighbour edge: no V2";
                rv = false;
            }
            if (!edges.contains(edge))
            {
                qWarning() << "Unknown edge" <<edgeIndex(edge) <<  "in neigbours list";
                rv = false;
            }
        }
    }

    return rv;
}
