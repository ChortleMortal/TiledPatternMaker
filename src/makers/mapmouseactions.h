#ifndef MAP_MOUSEACTIONS_H
#define MAP_MOUSEACTIONS_H

#include <QtWidgets>
#include "base/shared.h"
#include "makers/mapselection.h"

enum eMapMouseMode
{
    MAP_MODE_NONE,
    MAP_MODE_DRAW_LINE,
    MAP_MODE_DELETE,
    MAP_MODE_SPLIT_LINE,
    MAP_MODE_CONSTRUCTION_LINES,
    MAP_MODE_EXTEND_LINE,
    MAP_MODE_CONSTRUCTION_CIRCLES
};

#define E2STR(x) #x

static QString sMapMouseMode[]
{
    E2STR(MAP_MODE_NONE),
    E2STR(MAP_MODE_DRAW_LINE),
    E2STR(MAP_MODE_DELETE),
    E2STR(MAP_MODE_SPLIT_LINE),
    E2STR(MAP_MODE_CONSTRUCTION_LINES),
    E2STR(MAP_MODE_EXTEND_LINE),
    E2STR(MAP_MODE_CONSTRUCTION_CIRCLES)
};

class MapEditor;

class MapMouseAction
{
public:
    MapMouseAction(MapEditor * me, QPointF spt);
    virtual ~MapMouseAction() {}

    virtual void updateDragging(QPointF spt);
    virtual void draw(QPainter * painter);
    virtual void endDragging(QPointF spt);

    QString desc;

protected:
    QPointF     last_drag;
    MapEditor * me;
};

typedef shared_ptr<MapMouseAction> MapMouseActionPtr;

class MoveVertex : public MapMouseAction
{
public:
    MoveVertex(MapEditor * me, VertexPtr vp, QPointF spt);
    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

private:
    VertexPtr _vp;
};


class MoveEdge : public MapMouseAction
{
public:
    MoveEdge(MapEditor * me, EdgePtr edge, QPointF spt);
    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;

protected:
    EdgePtr _edge;
};

class DrawLine : public MapMouseAction
{
public:
    DrawLine(MapEditor * me, SelectionSet & set, QPointF spt);
    ~DrawLine() override;

    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

protected:
    QPointF * start;
    QPointF * end;
    VertexPtr startv;
    VertexPtr endv;

    QVector<QPointF> intersectPoints;
};

class ConstructionLine : public DrawLine
{
public:
    ConstructionLine(MapEditor * me, SelectionSet &, QPointF spt);

    virtual void endDragging( QPointF spt) override;

protected:
};

class ExtendLine : public MapMouseAction
{
public:
    ExtendLine(MapEditor * me, SelectionSet & set, QPointF spt);
    ~ExtendLine() override;

    void         flipDirection();

    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

protected:
    QLineF  startLine;
    QLineF  currentLine;
    EdgePtr startEdge;
    QPointF startDrag;

    QVector<QPointF> intersectPoints;
};

class MoveConstructionCircle : public MapMouseAction
{
public:
    MoveConstructionCircle(MapEditor * me, CirclePtr circle, QPointF spt);

    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

protected:
    CirclePtr origCircle;
    Circle    currentCircle;
};
#endif
