#ifndef MAPCLEANSER_H
#define MAPCLEANSER_H

#include <QtCore>
#include "base/shared.h"

class MapCleanser
{
public:
    MapCleanser(MapPtr map);
    ~MapCleanser();

    bool verifyMap(QString mapname, bool force = false);

    void cleanse();                 // does all of the fucntions below
    void removeBadEdges();
    void removeDanglingVertices();
    void removeBadEdgesFromNeighboursMap();
    void divideIntersectingEdges(); // if two edges cross, make a new vertex and have four edges

    void joinColinearEdges();       // if two lines are straight but have a vertex, then combine lines and delete vertex
    bool joinOneColinearEdge();
    void combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common);

    void cleanNeighbours();
    void fixNeighbours();

    void deDuplicateEdges(QVector<EdgePtr> & vec);
    void deDuplicateVertices(QVector<VertexPtr> & vec);
    void removeVerticesWithEdgeCount(int edgeCount);

protected:
    bool verifyVertices();
    bool verifyEdges();
    bool verifyNeighbours();

    void setTmpIndices() const;

private:
    MapPtr map;

    bool local;

    QDebug  * deb;
    QString astring;

    class Configuration * config;
};

#endif // MAPCLEANSER_H
