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

////////////////////////////////////////////////////////////////////////////
//
// Faces.java
//
// Here's where I pay the price for not implementing a complete
// doubly-connected edge list data structure.  When some piece
// of code finally wants to operate on the faces of a planar map,
// they aren't there.
//
// This class contains the algorithm to extract a collection of
// faces from a planar map.  It actually does a bit more than that.
// It assume the map represents a checkerboard diagram (i.e., every
// vertex has even degree except possibly for some at the borders
// of the map) and returns two arrays of faces, representing a
// two-colouring of the map.  The algorithm returns polygons because
// the only code that cares about faces doesn't need graph information
// about the faces, just the coordinates of their corners.

#include "geometry/faces.h"
#include "base/configuration.h"
#include "geometry/dcel.h"
#include "geometry/point.h"
#include "geometry/loose.h"
#include "geometry/map_cleanser.h"
#include "geometry/intersect.h"
#include "base/utilities.h"
#include <QPolygonF>

#undef  DELETE_LARGEST

#define E2STR(x) #x

static QString sFaceState[]
{
    E2STR(FACE_UNDONE),
    E2STR(FACE_PROCESSING),
    E2STR(FACE_BLACK),
    E2STR(FACE_WHITE),
    E2STR(FACE_DONE),
    E2STR(FACE_REMOVE)
};

////////////////////////////////////////
///
/// Face
///
////////////////////////////////////////

Face::Face()
{
    state   = FACE_UNDONE;
    outer   = false;
    color   = UNDEFINED;

    incident_edge   = nullptr;
    area            = -1;
}

QPolygonF Face::getPolygon()
{
    return getPoly();
}

QPointF Face::center()
{
    if (_center .isNull())
    {
        QPolygonF pts = getPolygon();
        _center = Point::center(pts);
    }
    return _center;
}

void Face::dump()
{
    qDebug() << "=== FACE";
    EdgePoly & ep = *this;
    ep.dump();
    qDebug() << "=== END FACE";
}


bool lessThanPoint(const QPointF &p1, const QPointF & p2)
{
    return ( p1.manhattanLength() < p2.manhattanLength());
}

bool Face::equals(FacePtr other)
{
    return getSorted() == other->getSorted();
}

QPolygonF & Face::getSorted()
{
    Q_ASSERT(sortedShadow.size() != 0);
    return sortedShadow;
}

void Face::sortForComparison()
{
    sortedShadow = getPoly();
    QVector<QPointF> & vec = sortedShadow;
    std::sort(vec.begin(),vec.end(),lessThanPoint);
}

bool  Face::overlaps(FacePtr other)
{
#if 0
    QPolygonF thisP   =  getPolygon();
    QPolygonF otherP  =  other->getPolygon();

    return thisP.intersects(otherP);
#else
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr ep1 = *it;
        QLineF l1 = ep1->getLine();
        for (auto it2 = other->begin(); it2 != other->end(); it2++)
        {
            EdgePtr ep2 = *it;
            QLineF l2   = ep2->getLine();
            QPointF intersect;
            if (Intersect::getTrueIntersection(l1,l2,intersect))
            {
                return true;
            }
        }
    }
    return false;
#endif
}

bool Face::containsCrossover()
{
    QVector<VertexPtr> qvec;
    for (auto edge : qAsConst(*this))
    {
        VertexPtr vp = edge->v1;
        if (!qvec.contains(vp))
        {
            qvec.push_back(vp);
        }
        else
        {
            //qDebug() << "crossover detected";
            return true;
        }
    }
    return false;
}

FaceSet Face::decompose()
{
    FaceSet fset;

    bool rv = false;
    do
    {
        FacePtr face = make_shared<Face>();
        rv = decomposeOnce(face);
        if (rv)
        {
            fset.push_back(face);
        }
    } while (rv);

    return fset;
}


bool Face::decomposeOnce(FacePtr newFace)
{
    enum eState
    {
        NOT_STARTED,
        STARTED,
        ENDED
    };

    QVector<VertexPtr> qvec;
    for (auto edge : qAsConst(*this))
    {
        VertexPtr vp = edge->v1;
        if (!qvec.contains(vp))
        {
            qvec.push_back(vp);
        }
        else
        {
            // create a good face and take it out of this face
            //qDebug() << "Crossover vertex detected" << vp->getTmpVertexIndex();
            Face master;
            eState state = NOT_STARTED;
            for (auto edge : qAsConst(*this))
            {
                switch(state)
                {
                case NOT_STARTED:
                    if (edge->v1 != vp)
                    {
                        master.push_back(edge);
                    }
                    else
                    {
                        state = STARTED;
                        newFace->push_back(edge);
                    }
                    break;
                case STARTED:
                    if (edge->v1 != vp)
                    {
                        newFace->push_back(edge);
                    }
                    else
                    {
                        state = ENDED;
                        master.push_back(edge);
                    }
                    break;
                case ENDED:
                    master.push_back(edge);
                    break;
                }
            }

            Q_ASSERT(newFace->isValid());
            Q_ASSERT(master.isValid());
            *this = master;
            return true;
        }
    }
    return false;
}

////////////////////////////////////////
///
/// Face Set
///
////////////////////////////////////////

void FaceSet::sortByPositon()
{
    newSet.clear();

    for (auto it = begin(); it != end(); it++)
    {
        FacePtr fp = *it;
        if (newSet.isEmpty())
        {
            newSet.push_back(fp);
        }
        else
        {
            sortByPositon(fp);
        }
    }

    clear();

    QVector<FacePtr> & faces = *this;
    faces = newSet;
}

void FaceSet::sortByPositon(FacePtr fp)
{
#if 1
    // sort by y-axis then x-axis
    QPointF pt = fp->center();
    for (int i =0; i < newSet.size(); i++)
    {
        // sort by y-axis
        FacePtr fp2 = newSet[i];
        QPointF pt2 = fp2->center();
        if (pt.y() < pt2.y())
        {
            newSet.insert(i,fp);   // insert before
            return;
        }
        else if (Loose::equals(pt.y(), pt2.y()) )
        {
            // sort by x-axis
            while (i < newSet.size())
            {
                fp2 = newSet[i];
                pt2 = fp2->center();
                if (!Loose::equals(pt.y(), pt2.y()))
                {
                    newSet.insert(i,fp);
                    return;
                }
                if (pt.x() < pt2.x())
                {
                    newSet.insert(i,fp);
                    return;
                }
                i++;
            }
            newSet.push_back(fp);
            return;
        }
    }
    newSet.push_back(fp);

#else
    // sort by x-axis then y-axis
    QPointF pt = fp->center();
    for (auto it = newSet.begin(); it != newSet.end(); it++)
    {
        // sort by x-axis
        FacePtr fp2 = *it;
        QPointF pt2 = fp2->center();
        if (pt.x() < pt2.x())
        {
            newSet.insert(it,fp);   // insert before
            return;
        }
        else if (Loose::equals(pt.x(), pt2.x()) )
        {
            // sort by y-axis
            while (it != newSet.end())
            {
                fp2 = *it;
                pt2 = fp2->center();
                if (!Loose::equals(pt.x(), pt2.x()))
                {
                    newSet.insert(it,fp);
                    return;
                }
                if (pt.y() < pt2.y())
                {
                    newSet.insert(it,fp);
                    return;
                }
                it++;
            }
            newSet.insert(it,fp);
            return;
        }
    }
    newSet.push_back(fp);
#endif
}

void FaceSet::dump(DCELPtr dcel)
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl;

    for (auto it = begin(); it != end(); it++)
    {
        FacePtr aface = *it;
        deb << "F" << dcel->faceIndex(aface);
        if (aface->outer)
            deb << " OUTER\t";
        else
            deb << "      \t";

        deb << " size = " << aface->area << "\t";

        dcelEdgePtr head = aface->incident_edge;
        if (!head)
        {
            deb << "NULL edge";
        }
        else
        {
            dcelEdgePtr e = head;
            do
            {
                dcel->print_edge(e,deb);
                e = e->next;
            } while (e != head);
        }
        deb << endl;
    }
    qDebug().noquote() << astring;
}

void FaceGroup::select(int idx)
{
    if (idx >= 0 && idx < size())
    {
        QVector<FaceSetPtr> & self = *this;

        for (int i=0; i < size(); i++)
        {
            FaceSetPtr fsp = self[i];
            if (i==idx)
                fsp->selected = true;
            else
                fsp->selected = false;
        }
    }
}

void FaceGroup::deselect()
{
    QVector<FaceSetPtr> & self = *this;
    for (int i=0; i < size(); i++)
    {
        FaceSetPtr fsp = self[i];
        fsp->selected = false;
    }
}

void FaceGroup::deselect(int idx)
{
    if (idx >= 0 && idx < size())
    {
        QVector<FaceSetPtr> & self = *this;
        FaceSetPtr fsp = self[idx];
        fsp->selected = false;
    }
}

bool FaceGroup::isSelected(int idx)
{
    if (idx >= 0 && idx < size())
    {
        QVector<FaceSetPtr> & self = *this;
        FaceSetPtr fsp = self[idx];
        return fsp->selected;
    }
    return false;
}

int FaceGroup::totalSize()
{
    int tot = 0;
    for (const auto & fset : qAsConst(*this))
    {
        tot += fset->size();
    }
    return tot;
}
