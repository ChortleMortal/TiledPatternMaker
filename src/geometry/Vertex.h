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

#include <memory>
#include <QPointF>
#include <QVector>
#include <QTransform>

typedef std::weak_ptr<class Vertex>         WeakVertexPtr;
typedef std::shared_ptr<class Edge>         EdgePtr;
typedef std::shared_ptr<class Vertex>       VertexPtr;

class Vertex
{
public:
    Vertex(const QPointF & pos);
    ~Vertex();

    qreal   getAngle(const EdgePtr & edge);
    void    applyRigidMotion(QTransform T);

    static int  refs;

    QPointF     pt;
    bool        visited;    // used by interlace
    WeakVertexPtr   copy;       // Used when cloning the map.
    QVector<WeakVertexPtr> adjacent_vertices;

};

#endif

