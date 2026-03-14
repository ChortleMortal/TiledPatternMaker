#pragma once
#ifndef FACES_H
#define FACES_H

#include <QHash>
#include <QHashFunctions>
#include "model/styles/colorset.h"
#include "sys/geometry/edge_poly.h"

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

class Face : public EdgeSet
{
public:
    Face();
    Face(EdgePoly & ep);
    ~Face() { refs--; }

    int         getNumSides() { return size(); }

    QPointF     center();
    QPolygonF   getPolygon();
    bool        contains(QPointF mpt);
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
    FaceSet() { selected = false; }

    static bool sort(const FaceSetPtr& d1, const FaceSetPtr& d2) { return (d1->area * d1->sides) > (d2->area * d2->sides); }
    void        sortByPositon();
    FacePtr     findByCenter(QPointF pt) const;
    void        dump(DCELPtr dcel);

    qreal       area;
    int         sides;     // all members of the group have same number of vertices
    bool        selected;  // volatile

protected:
    void sortByPositon(FacePtr fp, QVector<FacePtr> & newSet);
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

#endif
