#pragma once
#ifndef FACES_H
#define FACES_H

#include "sys/geometry/edgepoly.h"
#include "model/styles/colorset.h"

typedef std::shared_ptr<class Face>         FacePtr;
typedef std::shared_ptr<class FaceSet>      FaceSetPtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::shared_ptr<class DCEL>         DCELPtr;


class FaceSet;
class FaceGroups;

enum eFaceState
{
    FACE_UNDONE,
    FACE_PROCESSING,
    FACE_BLACK,
    FACE_WHITE,
    FACE_DONE,
    FACE_REMOVE
};

/////////////////////////////////////////////////
///
///  Face
///
/////////////////////////////////////////////////

class dcelEdge;

class Face : public EdgePoly
{
public:
    Face();
    Face(EdgePoly & ep);
    ~Face() { refs--; }

    int         getNumSides() { return size(); }
    QPolygonF   getPolygon();
    QPointF     center();

    bool        overlaps(FacePtr other);

    void        dump();

    bool        outer;
    WeakEdgePtr incident_edge;
    QColor      color;
    eFaceState  state;
    qreal       area;
    int         iPalette;   // palette color index

    static int  refs;

protected:

private:
    QPointF     _center;
};

/////////////////////////////////////////////////
///
///  FaceSet
///
/////////////////////////////////////////////////

class FaceSet : public QVector<FacePtr>
{
public:
    FaceSet() { selected = false; colorSet = nullptr; }
    qreal            area;
    TPColor          tpcolor;   // algo 0/1/2
    ColorSet *       colorSet;	// algo3
    int              sides;     // all members of the group have same number of vertices
    bool             selected;  // volatile

    static bool sort(const FaceSetPtr& d1, const FaceSetPtr& d2) { return (d1->area * d1->sides) > (d2->area * d2->sides); }

    void sortByPositon();

    void   dump(DCELPtr dcel);

protected:
    void sortByPositon(FacePtr fp, QVector<FacePtr> & newSet);

};


/////////////////////////////////////////////////
///
///  Face Group
///
/////////////////////////////////////////////////

class FaceGroups : public QVector<FaceSetPtr>
{
public:
    bool isSelected(int idx);
    void select(int idx);
    void deselect(int idx);
    void deselect();
    int  totalSize();
};

#endif

