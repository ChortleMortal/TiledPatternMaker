#pragma once

#include <QVector>
#include <QPointF>
#include "sys/geometry/circle.h"
#include "gui/map_editor/map_selection.h" // needed

class MapEditor;
class MapEditorDb;

typedef std::shared_ptr<class MapSelection>     MapSelectionPtr;
typedef QVector<MapSelectionPtr>                SelectionSet;
typedef QVector<EdgePtr>                        EdgeSet;

typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Circle>           CirclePtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Neighbours>       NeighboursPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class TileMotif>        TileMotifPtr;

class MapEditorSelection
{
public:
    MapEditorSelection(MapEditor * me);
    ~MapEditorSelection() {}

    void            setCurrentSelections(SelectionSet set)  { currentSelections = set; }
    SelectionSet    getCurrentSelections()                  { return currentSelections; }

    SelectionSet    findSelectionsUsingDB(const QPointF &spt);
    MapSelectionPtr findAPoint(SelectionSet & set);
    MapSelectionPtr findALine(SelectionSet & set);
    QPointF         snapPoint(SelectionSet & set, QPointF spt, bool * snapped = nullptr);   // model point: a point, else a point on a line, else spt
    MapSelectionPtr findAnEdge(SelectionSet & set);

    // points
    MapSelectionPtr findVertex(QPointF spt, VertexPtr exclude);

    // lines
    void            findEdges(MapPtr map, QPointF spt, const EdgeSet &excludes, SelectionSet & set);
    SelectionSet    findEdges(QPointF spt, const NeighboursPtr excludes);

    // circles
    MapSelectionPtr findConstructionCircle(const QPointF &spt);


    bool            insideBoundary(QPointF wpt);

private:
    SelectionSet currentSelections;

    MapEditor     * editor;
    MapEditorDb   * db;
};
