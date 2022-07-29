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
    MapSelection(QPointF p);
    MapSelection(EdgePtr e);
    MapSelection(QLineF  l, bool constructionLine = false);
    MapSelection(CirclePtr circle, bool constructionLine = false);
    MapSelection(CirclePtr circle, QPointF intersect, bool constructionLine = false);

    eMapSelection getType() { return _type; }
    VertexPtr     getVertex();
    EdgePtr       getEdge();
    QPointF       getPoint();
    CirclePtr     getCircle();
    QLineF        getLine();
    bool          isConstructionLine() { return _constructionLine; }
    bool          hasCircleIntersect() { return _hasCircleIntersect; }

    QPointF       getPointNear(MapSelectionPtr sel, QPointF pt);

private:
    eMapSelection    _type;
    VertexPtr        _vert;
    EdgePtr          _edge;
    QLineF           _line;
    QPointF          _pt;
    CirclePtr        _circle;
    bool             _constructionLine;
    bool             _hasCircleIntersect;
};

enum ePointInfo
{
    PT_VERTEX,
    PT_VERTEX_MID,
    PT_VERTEX_MID2,
    PT_LINE,
    PT_LINE_MID,
    PT_CIRCLE,
    PT_CIRCLE_1,
    PT_CIRCLE_2
};

class pointInfo
{
public:
    pointInfo() {}
    pointInfo(ePointInfo type, QPointF   p, QString desc = QString());
    pointInfo(ePointInfo type, VertexPtr v, QString desc = QString());
    ePointInfo      _type;
    QPointF         _pt;
    VertexPtr       _vert;
    QString         _desc;
};


enum eLineInfo
{
    LINE_EDGE,
    LINE_FIXED,
    LINE_CONSTRUCTION
};

class lineInfo
{
public:
    lineInfo() {}
    lineInfo(eLineInfo type, QLineF  l, QString desc = QString());
    lineInfo(eLineInfo type, EdgePtr e, QString desc = QString());

    eLineInfo       _type;
    QLineF          _line;
    EdgePtr         _edge;
    QString         _desc;
};
#endif

