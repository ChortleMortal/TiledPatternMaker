#pragma once
#ifndef RADIAL_RAY_H
#define RADIAL_RAY_H

#include <QPointF>
#include <QLineF>
#include <QVector>
#include <QTransform>

typedef std::shared_ptr<class Map>  MapPtr;

// This is the reay which is replicated to make the Radial Motif
// The tip is at the start
class RadialRay
{
public:
    RadialRay();

    void    clear();
    void    set(const QVector<QPointF> & vPoints);
    void    addTail(const QPointF point);
    void    addTip(const QPointF point);

    QPointF getTip() { return points[0]; }
    QPointF getTail(){ return points.last(); }
    QLineF  getRay() { return QLineF(points[1],points[0]); }

    const QVector<QPointF> & get() { return points; }
    MapPtr  getMap();
    void    addToMap(MapPtr map);

    void    transform(QTransform t);

    bool    valid()   { return (points.count() >= 2); }

    void    debug();
    QString info();

private:
    QVector<QPointF> points;
    bool    built;
    MapPtr  map;
};


class RaySet
{
public:
    RadialRay ray1;
    RadialRay ray2;

    bool valid()                    { return (ray1.valid() && ray2.valid()); }
    void clear()                    { ray1.clear(); ray2.clear(); }
    void transform(QTransform t)    { ray1.transform(t); ray2.transform(t); }
    void debug()                    { ray1.debug(); ray2.debug(); }
};

#endif

