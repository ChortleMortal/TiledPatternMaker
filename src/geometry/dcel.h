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

enum eColor
{
    C_WHITE = 0,
    C_BLACK = 1
};

class DCEL : public Map
{
    friend class ColorMaker;
    friend class FaceSet;

public:
    DCEL(MapPtr map);
    ~DCEL();

    void    buildDCEL();
    void    displayDCEL(int val);
    void    clean();

    FaceSet                 faces;

    static int refs;

protected:
    void    fill_half_edge_table();
    void    fill_face_table_inner_components();
    void    fill_half_edge_faces();

    void    createFace(EdgePtr head);

    void    print_vertices();
    void    print_edges();
    void    print_ordered_edges();
    void    print_faces();
    void    print_adj();
    void    print_edge(EdgePtr edge, QDebug & deb);
    void    print_edge_detail(EdgePtr e, QString name, QDebug & deb);

    void    print_neighbouring_faces(EdgePtr edge);
    void    print_faces_with_area_lessthan_threshhold(double threshhold_area);

    int     vertexIndex(VertexPtr v);
    int     faceIndex(FacePtr face);
    int     edgeIndex(EdgePtr edge);

    VertexPtr     validAdjacent(VertexPtr vert);
    EdgePtr       next_half_edge(EdgePtr current);

    EdgePtr       findEdge(VertexPtr start, VertexPtr end, bool expected = true);
    FacePtr       findOuterFace();

    FacePtr check_if_inside(QVector<VertexPtr> & verts);
    bool    check_if_point_is_inside(VertexPtr ver, QVector<VertexPtr> & key);
    bool    isInside(QPolygonF & polygon, QPointF p);

    bool    doIntersect(QPointF p1, QPointF q1, QPointF p2, QPointF q2);
    int     orientation(QPointF p, QPointF q, QPointF r);

    bool    onSegment(QPointF p, QPointF q, QPointF r);
    double  angle(QPointF p1, QPointF p2, QPointF p3);
    double  area_poly(QVector<VertexPtr> & key);

private:

};

#endif // DCEL_H
