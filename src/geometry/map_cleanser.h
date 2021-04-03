#ifndef MAPCLEANSER_H
#define MAPCLEANSER_H

#include <QtCore>
#include "base/shared.h"

enum eMCOptions
{
    badVertices_0               = 0x01,
    badVertices_1               = 0x02,
    badEdges                    = 0x04,
    badNeighbours               = 0x08,
    joinupColinearEdges         = 0x10,
    divideupIntersectingEdges   = 0x20,
    cleanupNeighbours           = 0x40,
};

#define default_cleanse (badEdges | badNeighbours | badVertices_0| cleanupNeighbours)

class MapCleanser
{
public:
    MapCleanser(MapPtr map);
    ~MapCleanser();

    bool cleanse(unsigned int options, bool forceVerify = true);                 // does all of the fucntions below

protected:
    void joinColinearEdges();       // if two lines are straight but have a vertex, then combine lines and delete vertex
    void divideIntersectingEdges(); // if two edges cross, make a new vertex and have four edges
    void removeVerticesWithEdgeCount(int edgeCount);
    void cleanNeighbours();

    void removeBadEdges();
    bool joinOneColinearEdge();
    void combineLinearEdges(EdgePtr a, EdgePtr b,VertexPtr common);
    void fixNeighbours();
    void deDuplicateEdges(const QVector<EdgePtr> &vec);

private:
    MapPtr map;

    QDebug  * deb;
    QString astring;
};

#endif // MAPCLEANSER_H
