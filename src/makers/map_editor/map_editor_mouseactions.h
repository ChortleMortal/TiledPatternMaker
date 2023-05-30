#pragma once
#ifndef MAP_EDITOR_MOUSEACTIONS_H
#define MAP_EDITOR_MOUSEACTIONS_H

#include "makers/map_editor/map_editor_selection.h"
#include "geometry/circle.h"

typedef std::shared_ptr<class Vertex>   VertexPtr;
typedef std::shared_ptr<class Edge>     EdgePtr;
typedef std::shared_ptr<class Crop>     CropPtr;

class QPainter;
class QKeyEvent;
class MapEditor;
class MapEditorDb;
class MapEditorView;
class ViewControl;

enum eCircleMode
{
    CM_OUTSIDE,
    CM_EDGE,
    CM_INSIDE
};

class MapMouseAction
{
public:
    MapMouseAction(QPointF spt);
    virtual ~MapMouseAction() {}

    virtual void updateDragging(QPointF spt);
    virtual void draw(QPainter * painter);
    virtual void endDragging(QPointF spt);

    QString desc;

protected:
    void    forceRedraw();

    QPointF              last_drag;
    MapEditorDb        * db;
    MapEditorView      * meView;
    MapEditorSelection * selector;
    ViewControl        * view;
};

typedef std::shared_ptr<MapMouseAction> MapMouseActionPtr;

class MoveVertex : public MapMouseAction
{
public:
    MoveVertex(VertexPtr vp, QPointF spt);
    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

private:
    VertexPtr _vp;
};


class MoveEdge : public MapMouseAction
{
public:
    MoveEdge(EdgePtr edge, QPointF spt);
    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;

protected:
    EdgePtr _edge;
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

class ExtendLine : public MapMouseAction
{
public:
    ExtendLine(SelectionSet & set, QPointF spt, bool isP1);
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

class EditConstructionCircle : public MapMouseAction
{
public:
    EditConstructionCircle(Circle circle, QPointF spt);

    virtual void updateDragging(QPointF spt) override;
    virtual void endDragging( QPointF spt) override;
    virtual void draw(QPainter * painter) override;

protected:
    Circle      origCircle;
    Circle      currentCircle;
    eCircleMode ecmode;
    QPointF   * start;
    QPointF   * end;
};



#endif
