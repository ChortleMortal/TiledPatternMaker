#include <QDebug>
#include "gui/map_editor/map_selection.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/geo.h"

PointInfo::PointInfo(Type type, QPointF pt)
{
    this->type = type;
    this->pt   = pt;
}

PointInfo::PointInfo(Type type, VertexPtr vert)
{
    this->type = type;
    this->vert = vert;
    this->pt   = vert->pt;
}

LineInfo::LineInfo(Type type, QLineF line)
{
    this->type = type;
    this->line = line;
}

LineInfo::LineInfo(Type type, EdgePtr edge)
{
    this->type = type;
    this->edge = edge;
    this->line = edge->getLine();
}


//////////////////////////////////////////////////
///
/// Map Selection
///
//////////////////////////////////////////////////

MapSelection::MapSelection(VertexPtr v)
{
    _type = Type::MAP_VERTEX;
    _vert = v;
    _pt   = v->pt;
    _constructionLine = false;
}

MapSelection::MapSelection(EdgePtr e)
{
    _type = Type::MAP_EDGE;
    _edge = e;
    _line = e->getLine();
    _constructionLine = false;
}

MapSelection::MapSelection(QLineF l, bool constructionLine)
{
    _type = Type::MAP_LINE;
    _line = l;
    _constructionLine = constructionLine;
}

MapSelection::MapSelection(QPointF p)
{
    _type = Type::MAP_POINT;
    _pt   = p;
    _constructionLine = false;
}

MapSelection::MapSelection(CirclePtr circle, bool constructionLine)
{
    _type               = Type::MAP_CIRCLE;
    _circle             = circle;
    _constructionLine   = constructionLine;
    _hasCircleIntersect = false;
}

MapSelection::MapSelection(CirclePtr circle, QPointF intersect, bool constructionLine)
{
    _type               = Type::MAP_CIRCLE;
    _circle             = circle;
    _pt                 = intersect;
    _constructionLine   = constructionLine;
    _hasCircleIntersect = true;
}

QPointF MapSelection::getPoint()
{
    QPointF apt;
    switch (_type)
    {
    case Type::MAP_VERTEX:
    case Type::MAP_POINT:
    case Type::MAP_CIRCLE:
        apt = _pt;
        break;

    case Type::MAP_EDGE:
    case Type::MAP_LINE:
        break;
    }
    return apt;
}


VertexPtr MapSelection::getVertex()
{
    return _vert;
}

EdgePtr MapSelection::getEdge()
{
    return _edge;
}

QLineF MapSelection::getLine()
{
    QLineF l;
    switch (_type)
    {
    case  Type::MAP_EDGE:
    case  Type::MAP_LINE:
        l = _line;
        break;

    case Type::MAP_VERTEX:
    case Type::MAP_POINT:
    case Type::MAP_CIRCLE:
        break;
    }
    return l;
}

CirclePtr MapSelection::getCircle()
{
    CirclePtr cp;
    if (_type == Type::MAP_CIRCLE)
    {
        cp = _circle;
    }
    return cp;
}

QPointF MapSelection::getPointNear(MapSelectionPtr sel, QPointF pt)
{
    Q_ASSERT(sel->getType() == Type::MAP_LINE || sel->getType() == Type::MAP_EDGE);
    QPointF apt = Geo::snapTo(pt,sel->getLine());
    return apt;
}



