#pragma once
#ifndef MAP_EDITOR_MOUSEACTIONS_H
#define MAP_EDITOR_MOUSEACTIONS_H

#include <QLineF>
#include <QObject>

#include "gui/map_editor/map_editor_selection.h"
#include "sys/geometry/circle.h"
#include "sys/geometry/neighbour_map.h"

typedef std::shared_ptr<class Vertex>   VertexPtr;
typedef std::shared_ptr<class Edge>     EdgePtr;
typedef std::shared_ptr<class Crop>     CropPtr;

class QPainter;
class QKeyEvent;
class MapEditor;
class MapEditorDb;
class MapEditorView;

enum eCircleMode
{
    CM_OUTSIDE,
    CM_EDGE,
    CM_INSIDE
};

class MapMouseAction : public QObject
{
    Q_OBJECT

public:
    MapMouseAction(QPointF spt);
    virtual ~MapMouseAction() {}

    virtual void updateDragging(QPointF spt);
    virtual void draw(QPainter * painter);
    virtual void endDragging(QPointF spt);

    QString desc;

signals:
    void    sig_updateView();

protected:
    void    forceRedraw();

    QPointF              last_drag;
    MapEditorDb        * db;
    MapEditorSelection * selector;
};

typedef std::shared_ptr<MapMouseAction> MapMouseActionPtr;

class MoveVertex : public MapMouseAction
{
public:
    MoveVertex(NeighbourMap * nmap, VertexPtr vp, QPointF spt);
    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

private:
    VertexPtr      _vp;
    NeighbourMap * nmap;
};


class MoveEdge : public MapMouseAction
{
public:
    MoveEdge(NeighbourMap * nmap, EdgePtr edge, QPointF spt);
    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;

protected:
    EdgePtr        _edge;
    NeighbourMap * nmap;
};

class DrawLine : public MapMouseAction
{
public:
    DrawLine(SelectionSet & set, QPointF spt);
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
    ConstructionLine(SelectionSet &, QPointF spt);

    virtual void endDragging( QPointF spt) override;

protected:
};

class ExtendLineP1 : public MapMouseAction
{
public:
    ExtendLineP1(SelectionSet & set, QPointF spt);
    ~ExtendLineP1() override;

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

class ExtendLineP2 : public MapMouseAction
{
public:
    ExtendLineP2(SelectionSet & set, QPointF spt);
    ~ExtendLineP2() override;

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

class EditConstructionCircle : public MapMouseAction
{
public:
    EditConstructionCircle(CirclePtr circle, QPointF spt);

    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

protected:
    CirclePtr   origCircle;
    Circle      currentCircle;
    eCircleMode ecmode;
    QPointF   * start;
    QPointF   * end;
};



#endif
