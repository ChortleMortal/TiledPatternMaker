#pragma once
#ifndef NEIGHBOURMAP_H
#define NEIGHBOURMAP_H

#include <QVector>
#include <QMap>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;
typedef std::shared_ptr<class Neighbours>   NeighboursPtr;

class NeighbourMap
{
public:
    explicit NeighbourMap(const QVector<EdgePtr> &edges);

    NeighboursPtr getNeighbours(const VertexPtr & v);

    uint rawSize();

protected:
    NeighboursPtr  getRawNeighbours(const VertexPtr & v);

private:
    QMap<VertexPtr,NeighboursPtr>   nmap;
};

#endif // NEIGHBOURMAP_H
