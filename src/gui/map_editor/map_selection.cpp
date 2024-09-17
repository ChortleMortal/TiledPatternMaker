#include <QDebug>
#include "gui/map_editor/map_selection.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/circle.h"
#include "sys/geometry/geo.h"

static bool debugInfoSelection = false;

PointInfo::PointInfo(ePointInfo type, QPointF pt, QString desc)
{
    this->type = type;
    this->pt   = pt;
    this->desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}

PointInfo::PointInfo(ePointInfo type, VertexPtr vert, QString desc)
{
    this->type = type;
    this->vert = vert;
    this->pt   = vert->pt;
    this->desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}

LineInfo::LineInfo(eLineInfo type, QLineF line, QString desc)
{
    this->type = type;
    this->line = line;
    this->desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}

LineInfo::LineInfo(eLineInfo type, EdgePtr edge, QString desc)
{
    this->type = type;
    this->edge = edge;
    this->line = edge->getLine();
    this->desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}


//////////////////////////////////////////////////
///
/// Map Selection
///
//////////////////////////////////////////////////

MapSelection::MapSelection(VertexPtr v)
{
    _type = MAP_VERTEX;
    _vert = v;
    _pt   = v->pt;
    _constructionLine = false;
}

MapSelection::MapSelection(EdgePtr e)
{
    _type = MAP_EDGE;
    _edge = e;
    _line = e->getLine();
    _constructionLine = false;
}

MapSelection::MapSelection(QLineF l, bool constructionLine)
{
    _type = MAP_LINE;
    _line = l;
    _constructionLine = constructionLine;
}

MapSelection::MapSelection(QPointF p)
{
    _type = MAP_POINT;
    _pt   = p;
    _constructionLine = false;
}

MapSelection::MapSelection(CirclePtr circle, bool constructionLine)
{
    _type               = MAP_CIRCLE;
    _circle             = circle;
    _constructionLine   = constructionLine;
    _hasCircleIntersect = false;
}

MapSelection::MapSelection(CirclePtr circle, QPointF intersect, bool constructionLine)
{
    _type               = MAP_CIRCLE;
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
    case MAP_VERTEX:
    case MAP_POINT:
    case MAP_CIRCLE:
        apt = _pt;
        break;

    case MAP_EDGE:
    case MAP_LINE:
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
    case  MAP_EDGE:
    case  MAP_LINE:
        l = _line;
        break;

    case MAP_VERTEX:
    case MAP_POINT:
    case MAP_CIRCLE:
        break;
    }
    return l;
}

CirclePtr MapSelection::getCircle()
{
    CirclePtr cp;
    if (_type == MAP_CIRCLE)
    {
        cp = _circle;
    }
    return cp;
}

QPointF MapSelection::getPointNear(MapSelectionPtr sel, QPointF pt)
{
    Q_ASSERT(sel->getType() == MAP_LINE || sel->getType() == MAP_EDGE);
    QPointF apt = Geo::snapTo(pt,sel->getLine());
    return apt;
}



