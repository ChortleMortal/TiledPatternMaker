#ifndef MAPEDITORSELCTION_H
#define MAPEDITORSELCTION_H

#include "viewers/map_editor_view.h"

class lineInfo;

typedef std::shared_ptr<class MapSelection> MapSelectionPtr;
typedef QVector<MapSelectionPtr> SelectionSet;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Neighbours>       NeighboursPtr;

class MapEditorSelection : public MapEditorView
{
public:
    void            buildEditorDB();

protected:
    MapEditorSelection();

    SelectionSet    findSelectionsUsingDB(const QPointF &spt);
    MapSelectionPtr findAPoint(SelectionSet & set);
    MapSelectionPtr findALine(SelectionSet & set);
    MapSelectionPtr findAnEdge(SelectionSet & set);

    // points
    MapSelectionPtr findVertex(QPointF spt, VertexPtr exclude);

    // lines
    SelectionSet    findEdges(QPointF spt, const QVector<EdgePtr> &excludes);
    SelectionSet    findEdges(QPointF spt, const NeighboursPtr excludes);

    // circles
    MapSelectionPtr findConstructionCircle(const QPointF &spt);

    bool            insideBoundary(QPointF wpt);

    QVector<lineInfo>  lines;   // generated
    QVector<pointInfo> points;  // generated
    QVector<CirclePtr> circles; // generated
};

#endif // MAPEDITORSELCTION_H
