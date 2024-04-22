#pragma once
#ifndef MAP_SELECTION_H
#define MAP_SELECTION_H

#include <QLineF>
#include <QString>
#include <QVector>
#include "geometry/circle.h"

enum eMapSelection
{
    MAP_VERTEX,
    MAP_POINT,
    MAP_EDGE,
    MAP_LINE,
    MAP_CIRCLE
};

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
    MapSelection(VertexPtr v);
    MapSelection(EdgePtr e);
    MapSelection(CirclePtr circle, bool constructionLine = false);
    MapSelection(CirclePtr circle, QPointF intersect, bool constructionLine = false);

    MapSelection(QPointF p);
    MapSelection(QLineF  l, bool constructionLine = false);

    eMapSelection getType() { return _type; }

    VertexPtr     getVertex();
    EdgePtr       getEdge();
    CirclePtr     getCircle();

    QPointF       getPoint();
    QLineF        getLine();

    bool          isConstructionLine() { return _constructionLine; }
    bool          hasCircleIntersect() { return _hasCircleIntersect; }

    QPointF       getPointNear(MapSelectionPtr sel, QPointF pt);

private:
    eMapSelection    _type;

    VertexPtr        _vert;
    EdgePtr          _edge;
    CirclePtr        _circle;

    QLineF           _line;
    QPointF          _pt;

    bool             _constructionLine;
    bool             _hasCircleIntersect;
};

enum ePointInfo
{
    PT_VERTEX,
    PT_VERTEX_MID,
    PT_LINE,
    PT_LINE_MID,
    PT_CIRCLE,
    PT_CIRCLE_1,
    PT_CIRCLE_2
};

class PointInfo
{
public:
    PointInfo() {}
    PointInfo(ePointInfo type, QPointF   pt,   QString desc = QString());
    PointInfo(ePointInfo type, VertexPtr vert, QString desc = QString());
    ePointInfo      type;
    QPointF         pt;
    VertexPtr       vert;
    QString         desc;
};


enum eLineInfo
{
    LINE_EDGE,
    LINE_FIXED,
    LINE_CONSTRUCTION
};

class LineInfo
{
public:
    LineInfo() {}
    LineInfo(eLineInfo type, QLineF  line, QString desc = QString());
    LineInfo(eLineInfo type, EdgePtr edge, QString desc = QString());

    eLineInfo       type;
    QLineF          line;
    EdgePtr         edge;
    QString         desc;
};
#endif

