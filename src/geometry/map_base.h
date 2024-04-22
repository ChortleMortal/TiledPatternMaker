#ifndef MAP_BASE_H
#define MAP_BASE_H

#include "misc/unique_qvector.h"
#include "geometry/vertex.h"

class MapBase
{
public:
    MapBase();

    void        paint(QPainter * painter, QTransform & tr, bool shoWDirn = false, bool showArcCenters = false, bool showVertices = false);

    // getters
    const QVector<VertexPtr>    & getVertices()         { return vertices; }
    const QVector<EdgePtr>      & getEdges()            { return edges; }
    const QVector<QPair<QPointF,QString>> & getTexts()  { return debugTexts; }

    void        transform(const QTransform &T);

    bool        isEmpty() const;
    void        wipeout();

protected:
    UniqueQVector<VertexPtr>         vertices;
    UniqueQVector<EdgePtr>           edges;
    QVector<QPair<QPointF,QString>>  debugTexts;

};

#endif // MAP_BASE_H
