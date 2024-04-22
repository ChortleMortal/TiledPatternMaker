#pragma once
#ifndef MAPEDITORSELCTION_H
#define MAPEDITORSELCTION_H

#include <QVector>
#include <QPointF>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif
#include "geometry/circle.h"

class LineInfo;
class PointInfo;
class Configuration;
class MapEditorDb;
class MapEditorView;

typedef std::shared_ptr<class MapSelection>     MapSelectionPtr;
typedef QVector<MapSelectionPtr>                SelectionSet;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class Circle>           CirclePtr;
typedef std::shared_ptr<class Edge>             EdgePtr;
typedef std::shared_ptr<class Neighbours>       NeighboursPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

class MapEditorSelection
{
public:
    MapEditorSelection(MapEditorDb *db);

    void            buildEditorDB();

    void            setCurrentSelections(SelectionSet set) { currentSelections = set; }
    SelectionSet    getCurrentSelections() { return currentSelections; }

    SelectionSet    findSelectionsUsingDB(const QPointF &spt);
    MapSelectionPtr findAPoint(SelectionSet & set);
    MapSelectionPtr findALine(SelectionSet & set);
    MapSelectionPtr findAnEdge(SelectionSet & set);

    // points
    MapSelectionPtr findVertex(QPointF spt, VertexPtr exclude);

    // lines
    void            findEdges(MapPtr map, QPointF spt, const QVector<EdgePtr> &excludes, SelectionSet & set);
    SelectionSet    findEdges(QPointF spt, const NeighboursPtr excludes);

    // circles
    MapSelectionPtr findConstructionCircle(const QPointF &spt);

    void            buildMotifDB(DesignElementPtr delp);
    bool            insideBoundary(QPointF wpt);

    QVector<LineInfo>  lines;   // generated
    QVector<PointInfo> points;  // generated
    QVector<CirclePtr> circles; // generated

private:
    SelectionSet currentSelections;

    Configuration * config;
    MapEditorDb   * db;
    MapEditorView * meView;
};

#endif // MAPEDITORSELCTION_H
