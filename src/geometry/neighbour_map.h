#ifndef NEIGHBOURMAP_H
#define NEIGHBOURMAP_H

#include <QVector>
#include <QMap>

typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;

class NeighbourMap
{
public:
    NeighbourMap(const QVector<EdgePtr> &edges);

    void build();
    void resetNmap();

    NeighboursPtr getNeighbours(const VertexPtr & v);

protected:
    NeighboursPtr  getRawNeighbours(const VertexPtr & v);

private:
    QMap<VertexPtr,NeighboursPtr>   nmap;
};

#endif // NEIGHBOURMAP_H
