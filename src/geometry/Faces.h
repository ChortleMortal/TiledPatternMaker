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

#include <QtCore>
#include "base/shared.h"
#include "geometry/edgepoly.h"
#include "base/colorset.h"

typedef std::shared_ptr<class Face>         FacePtr;
typedef std::shared_ptr<class FaceSet>      FaceSetPtr;
typedef std::weak_ptr<class Edge>           WeakEdgePtr;
typedef std::shared_ptr<class DCEL>         DCELPtr;


class FaceSet;
class FaceGroup;

enum eFaceState
{
    FACE_UNDONE,
    FACE_PROCESSING,
    FACE_BLACK,
    FACE_WHITE,
    FACE_DONE,
    FACE_REMOVE
};

enum eFaceColor
{
    UNDEFINED,
    BLACK,
    WHITE,
    RED,
    EXTERIOR
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
    ~Face() { refs--; }

    int         getNumSides() { return size(); }
    QPolygonF   getPolygon();
    QPointF     center();

    bool        overlaps(FacePtr other);

DAC_DEPRECATED  void    sortForComparison();
DAC_DEPRECATED  bool    equals(FacePtr other);
DAC_DEPRECATED  bool    containsCrossover();
DAC_DEPRECATED  FaceSet decompose();

    void        dump();

    bool        outer;
    WeakEdgePtr incident_edge;
    eFaceColor  color;
    eFaceState  state;
    qreal       area;

    static int refs;

protected:
DAC_DEPRECATED  bool    decomposeOnce(FacePtr newFace);
DAC_DEPRECATED  QPolygonF & getSorted();

private:
    QPointF     _center;

DAC_DEPRECATED  QPolygonF   sortedShadow;

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
    ColorSet       * pColorSet;	// algo3
    int              sides;     // all members of the group have same number of vertices
    bool             selected;  // volatile

    static bool sort(const FaceSetPtr& d1, const FaceSetPtr& d2) { return (d1->area * d1->sides) > (d2->area * d2->sides); }

    void sortByPositon();

    void   dump(DCELPtr dcel);

protected:
    void sortByPositon(FacePtr fp);

private:
    QVector<FacePtr> newSet;    // used in calcs
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

