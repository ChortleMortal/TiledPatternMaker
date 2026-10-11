#include <QDebug>
#include "model/motifs/radial_ray.h"
#include "sys/geometry/debug_map.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/map.h"

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

bool RaySet::equals(const RaySet& other) const
{
    const QVector<RadialRay>& a = *this;
    const QVector<RadialRay>& b = other;

    if (a.size() != b.size())
        return false;

    for (int i = 0; i < a.size(); ++i)
        if (!a[i].equals(b[i]))
            return false;

    return true;
}

void RadialRay::set(const RadialRay & vPoints)
{
    points = vPoints.get();
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

bool RadialRay::pointOnLineLessThan(QPointF origin, QPointF p1, QPointF p2)
{
    return  Geo::dist2(origin,p1) < Geo::dist2(origin,p2);
}

void RadialRay::sort(QPointF origin)
{
    std::sort(points.begin(), points.end(), [this,origin] (QPointF a, QPointF b) { return pointOnLineLessThan(origin,a, b); });
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

    for (QPointF & pt : points)
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
        VertexPtr v1 = map->insertVertex(points[i]);
        VertexPtr v2 = map->insertVertex(points[i+1]);
        map->insertEdge(v1,v2);
    }
}

void RadialRay::debug(int idx)
{
    qInfo().noquote() << info();
    uint idx2 = 0;
    for (QPointF & pt : points)
    {
        Sys::debugMapCreate->insertDebugMark(pt,QString("%1pt%2").arg(idx).arg(idx2++),Qt::red);
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

////////////////////////////////////////////////////////////////////////////
//
//  Ray Set
//
////////////////////////////////////////////////////////////////////////////

bool RaySet::valid()
{
    for (RadialRay & ray : *this)
    {
        if (!ray.valid())
            return false;
    }
    return true;
}

void RaySet::transform(QTransform t)
{
    for (RadialRay & ray : *this)
    {
        ray.transform(t);
    }
}

void RaySet::debug(int type)
{
    switch (type)
    {
    case 0:
    {
        int idx = 0;
        qDebug() << size() << "rays in set";
        for (RadialRay & ray : *this)
        {
            ray.debug(idx++);
        }
    }   break;

    case 1:
    {
        QString str;
        int i=0;
        for (RadialRay & ray : *this)
        {
            str += QString(" %1-%2").arg(i).arg(ray.size());
            i++;
        }
        qDebug().noquote() << str;
    }   break;

    default:
        break;
    }
}

QPointF RaySet::furthestPointFromOrigin() const
{
    QPointF bestPoint;
    double bestDist2 = -1.0;

    for (const RadialRay& ray : get())
    {
        const auto & pts = ray.get();   // adjust to your actual API

        for (const QPointF& p : pts)
        {
            double d2 = p.x()*p.x() + p.y()*p.y();
            if (d2 > bestDist2)
            {
                bestDist2 = d2;
                bestPoint = p;
            }
        }
    }

    return bestPoint;
}