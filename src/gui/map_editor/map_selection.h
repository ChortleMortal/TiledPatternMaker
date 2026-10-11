#pragma once

#include <QLineF>
#include <QString>
#include <QVector>

#include "sys/geometry/circle.h"

#define E2STR(x) #x
static QString sMapSelection[]
{
    E2STR(MAP_VERTEX),
    E2STR(MAP_POINT),
    E2STR(MAP_EDGE),
    E2STR(MAP_LINE),
    E2STR(MAP_CIRCLE)
};

typedef std::shared_ptr<class MapSelection> MapSelectionPtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::shared_ptr<class Circle>       CirclePtr;

typedef QVector<MapSelectionPtr>            SelectionSet;


class MapSelection
{
public:

    enum class Type
    {
        MAP_VERTEX,
        MAP_POINT,
        MAP_EDGE,
        MAP_LINE,
        MAP_CIRCLE
    };

    MapSelection(VertexPtr v);
    MapSelection(EdgePtr e);
    MapSelection(CirclePtr circle, bool constructionLine = false);
    MapSelection(CirclePtr circle, QPointF intersect, bool constructionLine = false);

    MapSelection(QPointF p);
    MapSelection(QLineF  l, bool constructionLine = false);

    Type        getType() { return _type; }

    VertexPtr   getVertex();
    EdgePtr     getEdge();
    CirclePtr   getCircle();

    QPointF     getPoint();
    QLineF      getLine();

    bool        isConstructionLine() { return _constructionLine; }
    bool        hasCircleIntersect() { return _hasCircleIntersect; }

    QPointF     getPointNear(MapSelectionPtr sel, QPointF pt);

private:
    Type        _type;

    VertexPtr   _vert;
    EdgePtr     _edge;
    CirclePtr   _circle;

    QLineF      _line;
    QPointF     _pt;

    bool        _constructionLine;
    bool        _hasCircleIntersect;
};


class PointInfo
{
public:

    enum class Type
    {
        Vertex,
        VertexMid,
        Line,
        LineMid,
        Circle,
        Circle1,    // touch point p1
        Circle2,    // touch point p2
        Center,
        Center2,

    };

    PointInfo() {}
    PointInfo(Type type, QPointF   pt);
    PointInfo(Type type, VertexPtr vert);

    Type        type;
    QPointF     pt;
    VertexPtr   vert;
};


class LineInfo
{
public:

    enum class Type
    {
        LINE_EDGE,
        LINE_FIXED,
        LINE_CONSTRUCTION
    };

    LineInfo() {}
    LineInfo(Type type, QLineF  line);
    LineInfo(Type type, EdgePtr edge);

    Type        type;
    QLineF      line;
    EdgePtr     edge;
};

