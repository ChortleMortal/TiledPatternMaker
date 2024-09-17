#include <QDebug>
#include "model/motifs/radial_ray.h"
#include "sys/geometry/map.h"
#include "gui/viewers/debug_view.h"

using std::make_shared;

////////////////////////////////////////////////////////////////////////////
//
//  Radial Ray
//
////////////////////////////////////////////////////////////////////////////

RadialRay::RadialRay()
{
    built = false;
}

void RadialRay::clear()
{
    points.clear();
    map.reset();
    built = false;
}

void RadialRay::set(const QVector<QPointF> & vPoints)
{
    points = vPoints;
    built  = false;
}

void RadialRay::addTail(const QPointF point)
{
    points.push_back(point);
    built  = false;
}

void RadialRay::addTip(const QPointF point)
{
    if (points.empty() || point != points.front())
    {
        points.push_front(point);
        built  = false;
    }
}

void RadialRay::transform(QTransform t)
{
    for (int i = 0; i < points.size(); i++)
    {
        points[i] = t.map(points[i]);
    }
}

MapPtr RadialRay::getMap()
{
    if (built)
    {
        return map;
    }

    map = make_shared<Map>("RadialRay");

    for (QPointF pt : points)
    {
        map->insertVertex(pt);
    }

    auto it = map->getVertices().begin();
    VertexPtr v1 = *it;
    while (it != map->getVertices().end())
    {
        it++;
        if (it == map->getVertices().end())
        {
            continue;
        }
        VertexPtr v2 = *it;
        map->insertEdge(v1,v2);
        v1 = v2;
    }

    built = true;
    return map;
}

void RadialRay::addToMap(MapPtr map)
{
    for (int i = 0; i < points.size() -1; i++)
    {
        map->insertEdge(points[i],points[i+1]);
    }
}

void RadialRay::debug()
{
    qInfo().noquote() << info();
    DebugMap * map = Sys::debugView->getMap();
    uint idx = 0;
    for (QPointF pt : points)
    {
        map->insertDebugMark(pt,QString("pt%1").arg(idx++));
    }
}

QString RadialRay::info()
{
    QString str;
    QDebug  deb(&str);

    deb << "RadialRay size=" << points.size();
    for (int i=0; i < points.size(); i++)
    {
        deb << points[i];
    }

    return str;
}
