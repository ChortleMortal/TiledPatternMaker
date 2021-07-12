/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAP_SELECTION_H
#define MAP_SELECTION_H

#include <QLineF>
#include <QString>
#include <QVector>

typedef std::shared_ptr<class Circle>  CirclePtr;

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
typedef QVector<MapSelectionPtr> SelectionSet;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;

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
    PT_LINE,
    PT_LINE_MID,
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

