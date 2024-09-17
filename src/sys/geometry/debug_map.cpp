#include "sys/geometry/debug_map.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/loose.h"
#include "gui/viewers/debug_view.h"

using std::make_shared;

/////////////////////////////////////////
///
/// DebugMap - we dont care about duplication and inefficiencies here.
/// We just want to put in the edges and vetices
///
//////////////////////////////////////////

DebugMap::DebugMap()
{
    name = "DebugMap";
}

DebugMap::DebugMap(const Map * map)
{
    MapPtr newMap   = map->recreate();
    name            = newMap->name();
    vertices        = newMap->vertices;
    edges           = newMap->edges;
    debugTexts      = newMap->debugTexts;
}

void DebugMap::insertDebugMark(QPointF m, QString txt)
{
    if (!Sys::debugView->getShow())
        return;

    qreal x = m.x();
    qreal y = m.y();

    qreal size = 0.05;
    QPointF p1(x-size,y);
    QPointF p2(x+size,y);
    QPointF p3(x,y+size);
    QPointF p4(x,y-size);

    VertexPtr v1 = make_shared<Vertex>(p1);
    VertexPtr v2 = make_shared<Vertex>(p2);
    insertEdge(v1,v2);

    v1 = make_shared<Vertex>(p3);
    v2 = make_shared<Vertex>(p4);
    insertEdge(v1,v2);

    if (txt.isEmpty())
        return;

    for (auto & pair : debugTexts)
    {
        if (Loose::equalsPt(pair.first,m))
        {
            pair.second += "  ";
            pair.second += txt;
            return;
        }
    }
    debugTexts.push_back(QPair<QPointF,QString>(m,txt));
}

void DebugMap::insertDebugLine(QLineF l1)
{
    insertDebugLine(l1.p1(),l1.p2());
}

void DebugMap::insertDebugEdge(EdgePtr edge)
{
    if (!Sys::debugView->getShow())
        return;

    // this is a dirty direct insert
    vertices.push_back(edge->v1);
    vertices.push_back(edge->v2);
    edges.push_back(edge);
}

void DebugMap::insertDebugLine(QPointF p1, QPointF p2)
{
    if (!Sys::debugView->getShow())
        return;

    VertexPtr v1 = make_shared<Vertex>(p1);
    VertexPtr v2 = make_shared<Vertex>(p2);
    insertEdge(v1,v2);
}

void DebugMap::insertDebugPoints(QPolygonF & poly)
{
    if (!Sys::debugView->getShow())
        return;

    for (int i=0; i < poly.size() -1; i++)
    {
        QPointF p1 = poly.at(i);
        QPointF p2 = poly.at(i+1);
        VertexPtr v1 = make_shared<Vertex>(p1);
        VertexPtr v2 = make_shared<Vertex>(p2);
        insertEdge(v1,v2);
    }
}

void DebugMap::insertDebugPolygon(QPolygonF & poly)
{
    if (!Sys::debugView->getShow())
        return;

    insertDebugPoints(poly);
    if (!poly.isClosed())
    {
        QPointF p1 = poly.at(poly.size()-1);
        QPointF p2 = poly.at(0);
        VertexPtr v1 = make_shared<Vertex>(p1);
        VertexPtr v2 = make_shared<Vertex>(p2);
        insertEdge(v1,v2);
    }
}

void  DebugMap::insertEdge(VertexPtr v1,VertexPtr v2)
{
    // quick and dirty
    vertices.push_back(v1);
    vertices.push_back(v2);
    auto edge = make_shared<Edge>(v1,v2);
    edges.push_back(edge);
}

QString DebugMap::info()
{
    QString astring = QString("vertices=%1 edges=%2 marks=%3").arg(vertices.count()).arg(edges.count()).arg(debugTexts.count());
    return astring;
}
