/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FACES_H
#define FACES_H

#include "geometry/vertex.h"
#include "geometry/map.h"
#include "base/shared.h"
#include "base/colorset.h"
#include <QtCore>

class Face;
class FaceSet;
class FaceGroup;

enum eFaceState
{
    FACE_UNDONE,
    FACE_PROCESSING,
    FACE_DONE_BLACK,
    FACE_DONE_WHITE,
    FACE_DONE,
    FACE_REMOVE
};

/////////////////////////////////////////////////
///
///  Face
///
/////////////////////////////////////////////////

class Face : public EdgePoly
{
public:
    Face();

    qreal     getArea();
    int       getNumSides() { return size(); }

    QPolygonF getPolygon();

    bool      overlaps(FacePtr other);
    QPolygonF subtracted(FacePtr other);

    bool      equals(FacePtr other);
    void      sortForComparison();
    QPolygonF &getSorted();

    bool      isClosed() { return first()->getV1() == last()->getV2(); }
    bool      containsCrossover();
    FaceSet   decompose();

    QPointF   center();

    eFaceState  state;

    void       dump();

protected:
    bool       decomposeOnce(FacePtr newFace);

private:
    QPointF     _center;
    qreal       _area;
    bool        _areaCalculated;
    QPolygonF   sortedShadow;
};

/////////////////////////////////////////////////
///
///  FaceSet
///
/////////////////////////////////////////////////

class FaceSet : public QVector<FacePtr>
{
public:
    FaceSet() { selected = false;}
    qreal            area;
    TPColor          tpcolor;   // algo 0/1/2
    ColorSet         colorSet;	// algo3
    int              sides;     // all members of the group have same number of vertices
    bool             selected;  // volatile

    static bool sort(const FaceSetPtr& d1, const FaceSetPtr& d2) { return (d1->area * d1->sides) > (d2->area * d2->sides); }

    void sortByPositon();

    QVector<FacePtr> newSet;    // used in calcs

protected:
    void sortByPositon(FacePtr fp);
};

/////////////////////////////////////////////////
///
///  Face Group
///
/////////////////////////////////////////////////

class FaceGroup : public QVector<FaceSetPtr>
{
public:
    bool isSelected(int idx);
    void select(int idx);
    void deselect(int idx);
    void deselect();
    int  totalSize();
};

/////////////////////////////////////////////////
///
///  Faces
///
/////////////////////////////////////////////////

class Faces
{
    enum eColor
    {
        C_WHITE = 0,
        C_BLACK = 1
    };

    friend class Filled;

public:
    Faces();

    void        buildFacesOriginal(MapPtr map);
    void        buildFacesNew23();

    void        assignColorsOriginal(MapPtr map);
    void        assignColorsNew1();
    void        assignColorsNew2(ColorSet & colorSet);
    void        assignColorsNew3(ColorGroup & colorGroup);

    void        purifyMap(MapPtr map);

    FaceGroup       & getFaceGroup()  { return faceGroup; }
    const FaceSet   & getAllFaces()   { return allFaces; }
    const FaceSet   & getWhiteFaces() { return whiteFaces; }
    const FaceSet   & getBlackFaces() { return blackFaces; }

protected:
    void        clearFaces();
    void        handleVertex(NeighbourMap & nmap, VertexPtr vert, EdgePtr edge);

    FacePtr     extractFace(NeighbourMap & nmap, VertexPtr from, EdgePtr edge);
    FacePtr     getTwin(MapPtr map, EdgePtr edge);
    void        assignColorsToFaces(MapPtr map, FaceSet & fset);
    void        addFaceResults(FaceSet & fst);

    void        dumpAllFaces(QString title);
    void        dumpFaceGroup(QString title);
    void        dumpEdgeFaceMap(QMap<EdgePtr,FacePtr> & dmap, QString title);

    void        sortFaceSetsByPosition();
    void        removeLargest();
    void        removeOverlaps();
    void        removeDuplicates();
    void        decomposeCrossoverFaces();
    void        zapFaceStates(enum eFaceState state);

    // Internal representations of the rendering.
    // Inside and outside (even/odd) regions.
    QMap<EdgePtr,FacePtr>   v1;
    QMap<EdgePtr,FacePtr>   v2;
    FaceSet                 allFaces;
    FaceSet                 whiteFaces;
    FaceSet                 blackFaces;
    FaceGroup               faceGroup;

private:
    qreal   determinant(QPointF vec1, QPointF vec2);
    bool    edgeIntersection(QPointF a, QPointF b, QPointF c, QPointF d);
    bool    isOverlapped(const FacePtr a, const FacePtr b);
};

#endif

