/*
MIT License

Copyright (c) 2018 Ankur Jaiswal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Taken from:  https://github.com/AnkurRyder/DCEL.git
*/

#ifndef DCEL_H
#define DCEL_H

#include <QtCore>
#include <base/shared.h>
#include "geometry/map.h"
#include "geometry/faces.h"

enum eVstate
{
    ES_UNDEFINED,
    ES_BOUNDED,
    ES_UNBOUNDED
};

enum eColor
{
    C_WHITE = 0,
    C_BLACK = 1
};

class dcelVertex
{
public:
    dcelVertex(VertexPtr v)  { vert = v; bstate = ES_UNDEFINED; incident_edge = nullptr; }

    QVector<dcelVertexPtr> adjacent_vertices;
    dcelEdgePtr      incident_edge;
    eVstate          bstate;
    VertexPtr        vert;  // the source
};


class dcelEdge
{
public:
    dcelEdge(EdgePtr ep) {edge = ep;  visited = false; }

    bool operator < (const dcelEdge & other) const;

    double angle() const { return std::atan2(v2->vert->pt.y() - v1->vert->pt.y(),
                                             v2->vert->pt.x() - v1->vert->pt.x()); }

    dcelVertexPtr  v1;
    dcelVertexPtr  v2;
    dcelEdgePtr    twin;
    dcelEdgePtr    next;
    dcelEdgePtr    prev() { return twin->next; }
    FacePtr        incident_face;
    EdgePtr        edge; // the source
    bool           visited;
};

class DCEL
{
    friend class ColorMaker;
    friend class FaceSet;

public:
    DCEL(Map * map);
    ~DCEL();

    void    buildDCEL(Map * map);
    void    displayDCEL(int val);
    void    clean();

    QVector<dcelVertexPtr>  vertices;
    QVector<dcelEdgePtr>    edges;
    FaceSet                 faces;


protected:
    void    fill_vertex_table();
    bool    fill_vertex_table2();
    void    fill_half_edge_table();
    void    fill_face_table_inner_components();
    void    fill_half_edge_faces();

    void    createFace(dcelEdgePtr head);

    void    print_vertices();
    void    print_edges();
    void    print_ordered_edges();
    void    print_faces();
    void    print_adj();
    void    print_edge(dcelEdgePtr edge, QDebug & deb);
    void    print_edge_detail(dcelEdgePtr e, QString name, QDebug & deb);

    void    print_neighbouring_faces(dcelEdgePtr edge);
    void    print_faces_with_area_lessthan_threshhold(double threshhold_area);

    int     vertexIndex(dcelVertexPtr v);
    int     faceIndex(FacePtr face);
    int     edgeIndex(dcelEdgePtr edge);

    dcelVertexPtr validAdjacent(dcelVertexPtr vert);
    dcelEdgePtr   next_half_edge(dcelEdgePtr current);

    dcelVertexPtr findVertex(VertexPtr v);
    dcelEdgePtr   findEdge(dcelVertexPtr start, dcelVertexPtr end, bool expected = true);
    FacePtr       findOuterFace();

    FacePtr check_if_inside(QVector<dcelVertexPtr> & verts);
    bool    check_if_point_is_inside(dcelVertexPtr ver, QVector<dcelVertexPtr> & key);
    bool    isInside(QPolygonF & polygon, QPointF p);

    bool    doIntersect(QPointF p1, QPointF q1, QPointF p2, QPointF q2);
    int     orientation(QPointF p, QPointF q, QPointF r);

    bool    onSegment(QPointF p, QPointF q, QPointF r);
    double  angle(QPointF p1, QPointF p2, QPointF p3);
    double  area_poly(QVector<dcelVertexPtr> & key);

};

#endif // DCEL_H
