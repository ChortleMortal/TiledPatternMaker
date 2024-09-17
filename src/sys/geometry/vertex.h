#pragma once
#ifndef VERTEX_H
#define VERTEX_H

////////////////////////////////////////////////////////////////////////////
//
// Vertex.java
//
// The vertex abstraction for planar maps.  A Vertex has the usual graph
// component, a list of adjacent edges.  It also has the planar component,
// a position.  Finally, there's a user data field for applications.

#include <QPointF>
#include <QVector>
#include <QTransform>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

typedef std::weak_ptr<class Vertex>         WeakVertexPtr;
typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;

class Vertex
{
public:
    Vertex(const QPointF & pos);
    ~Vertex();

    void inline setPt(QPointF p)   { pt = p; }

    qreal   getAngle(const EdgePtr & edge);
    void    transform(QTransform T);

public:
    QPointF         pt;
    bool            visited;    // used by interlace
    WeakVertexPtr   copy;       // Used when cloning the map.

    QVector<WeakVertexPtr> adjacent_vertices;

    static int  refs;
};

#endif

