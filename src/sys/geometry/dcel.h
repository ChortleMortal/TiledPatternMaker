#pragma once
#ifndef DCEL_H
#define DCEL_H

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

#include "sys/geometry/map.h"
#include "sys/geometry/faces.h"
#include "sys/geometry/neighbours.h"
#include "sys/geometry/neighbour_map.h"

typedef std::shared_ptr<class Face>         FacePtr;
typedef std::shared_ptr<class NeighbourMap> NeighbourMapPtr;

enum eColor
{
    C_WHITE = 0,
    C_BLACK = 1
};

//////////////////////////////////////////////////////////////////////////////
///
///  DCEL
///
//////////////////////////////////////////////////////////////////////////////

class DCEL : public Map
{
    friend class ColorMaker;
    friend class FaceSet;

public:
    DCEL(Map * sourcemap);
    virtual ~DCEL();

    void    displayDCEL(int val);

    FaceSet                  & getFaceSet()  { return faces; }
    const QVector<VertexPtr> & getVertices() { return vertices; }
    const QVector<EdgePtr>   & getEdges()    { return edges; }

    virtual QString info() const override;

    static int refs;

protected:
    void    buildDCEL();
    void    cleanseEdges();
    void    fill_half_edge_table();
    void    fill_face_table_inner_components();
    void    fill_half_edge_faces();

    EdgePtr next_half_edge(const EdgePtr & edge);
    EdgePtr findEdge(const VertexPtr & start, const VertexPtr &end, bool expected = true);

    void    createFace(const EdgePtr & head);
    FacePtr findOuterFace();
    FacePtr check_if_inside(const QVector<VertexPtr> & verts);

    double  angle(const QPointF & p1, const QPointF & p2, const QPointF & p3);
    double  area_poly(const QVector<VertexPtr> & key);

    int     orientation(const QPointF & p, const QPointF & q, const QPointF & r);

    bool    check_if_point_is_inside(const VertexPtr & ver, const QVector<VertexPtr> & key);
    bool    isInside(const QPolygonF & polygon, const QPointF & p);
    bool    doIntersect(const QPointF & p1, const QPointF & q1, const QPointF & p2, const QPointF & q2);
    bool    onSegment(const QPointF & p, const QPointF & q, const QPointF & r);

    void    print_vertices();
    void    print_edges();
    void    print_ordered_edges();
    void    print_faces();
    void    print_adj();
    void    print_edge(const EdgePtr & edge, QDebug & deb);
    void    print_edge_detail(const EdgePtr & e, QString && name, QDebug &deb);

    void    print_neighbouring_faces(const EdgePtr & edge);
    void    print_faces_with_area_lessthan_threshhold(double threshhold_area);

    VertexPtr validAdjacent(const VertexPtr & vert);

protected:
    int     faceIndex(const FacePtr & face) { return faces.indexOf(face); }

private:
    FaceSet faces;
    Map *   sourcemap;
};


#endif // DCEL_H
