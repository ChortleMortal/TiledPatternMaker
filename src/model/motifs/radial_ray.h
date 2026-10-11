#pragma once

#include <QPointF>
#include <QLineF>
#include <QTransform>
#include <QDebug>
#include "sys/qt/unique_qvector.h"

typedef std::shared_ptr<class Map>  MapPtr;

// This is the reay which is replicated to make the Radial Motif
// The tip is at the start
class RadialRay
{
public:
    RadialRay();

    bool    equals(const RadialRay& other) const { return points == other.points; }

    void    clear();
    void    set(const RadialRay &vPoints);
    void    addTail(const QPointF point);
    void    addTip(const QPointF point);
    void    setPt(int idx, QPointF pt)  { if (idx < points.size()) { points[idx] = pt; built = false; } }

    QPointF getTip()    { return points[0]; }
    QPointF getTail()   { return points.last(); }
    QPointF pt(int idx) { if (idx < points.size()) return points[idx]; else return points.last(); }
    QLineF  getRayTip() { return QLineF(points[1],points[0]); }

    void    sort(QPointF origin);

    const QVector<QPointF> & get() const { return points; }

    MapPtr  getMap();
    void    addToMap(MapPtr map);

    void    transform(QTransform t);

    void    resize(int n)   { points.resize(n);}
    int     size()          { return points.count(); }
    bool    valid()         { return (points.count() >= 2); }

    void    debug(int i);
    QString info();

protected:
    bool    pointOnLineLessThan(QPointF origin, QPointF p1, QPointF v2);

private:
    UniqueQVector<QPointF> points;
    bool    built;
    MapPtr  map;
};

class RaySet : private QVector<RadialRay>
{
public:
    using Base = QVector<RadialRay>;

    bool equals(const RaySet & other) const;

    void add(const RadialRay & r) { QVector<RadialRay>::push_back(r); }
    void removeAt(int i)          { QVector<RadialRay>::removeAt(i); }

    RadialRay & operator[](int i)       { return QVector<RadialRay>::operator[](i % size()); }
    const RadialRay & operator[](int i) const { return QVector<RadialRay>::operator[](i % size()); }

    QVector<RadialRay>       & get()        { return *this; }
    const QVector<RadialRay> & get() const  { return *this; }

    QPointF furthestPointFromOrigin() const;

    int size() const { return Base::size(); }
    void clear()     { Base::clear(); }

    bool valid();
    void transform(QTransform t);
    void debug(int type);

    int unextendedSize;     // number of rays before extensions are added
    int cadencePairs;       // number of KR ray pairs (sides) in unit ray
};
