#include "makers/map_editor/map_selection.h"
#include "geometry/edge.h"
#include <QDebug>
#include "geometry/vertex.h"
#include "geometry/geo.h"

static bool debugInfoSelection = false;

pointInfo::pointInfo(ePointInfo type, QPointF p, QString desc)
{
    _type = type;
    _pt   = p;
    _desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}

pointInfo::pointInfo(ePointInfo type, VertexPtr v, QString desc)
{
    _type = type;
    _vert = v;
    _pt   = v->pt;
    _desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}

lineInfo::lineInfo(eLineInfo type, QLineF l, QString desc)
{
    _type = type;
    _line = l;
    _desc = desc;
    if (debugInfoSelection) qDebug() << desc;
}

lineInfo::lineInfo(eLineInfo type, EdgePtr e, QString desc)
{
    _type = type;
    _edge = e;
    _line = e->getLine();
    _desc = desc;
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

MapSelection::MapSelection(Circle circle, bool constructionLine)
{
    _type               = MAP_CIRCLE;
    _circle             = circle;
    _constructionLine   = constructionLine;
    _hasCircleIntersect = false;
}

MapSelection::MapSelection(Circle circle, QPointF intersect, bool constructionLine)
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

Circle MapSelection::getCircle()
{
    Circle c;

    switch (_type)
    {
    case MAP_CIRCLE:
        c  = _circle;
        break;

    case MAP_EDGE:
    case MAP_LINE:
    case MAP_VERTEX:
    case MAP_POINT:
        break;
    }

    return c;
}

QPointF MapSelection::getPointNear(MapSelectionPtr sel, QPointF pt)
{
    Q_ASSERT(sel->getType() == MAP_LINE || sel->getType() == MAP_EDGE);
    QPointF apt = Geo::snapTo(pt,sel->getLine());
    return apt;
}



