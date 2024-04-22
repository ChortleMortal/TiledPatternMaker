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

#include <QDebug>

#include "geometry/faces.h"
#include "geometry/edge.h"
#include "geometry/dcel.h"
#include "geometry/geo.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "misc/tpm_io.h"

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

int Face::refs = 0;

////////////////////////////////////////
///
/// Face
///
////////////////////////////////////////

Face::Face()
{
    state    = FACE_UNDONE;
    outer    = false;
    area     = -1;
    iPalette = -1;
    refs++;
}

Face::Face(EdgePoly & ep) : EdgePoly(ep)
{
    state    = FACE_UNDONE;
    outer    = false;
    area     = -1;
    iPalette = -1;
    refs++;
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
        _center = Geo::center(pts);
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

bool  Face::overlaps(FacePtr other)
{
#if 0
    QPolygonF thisP   =  getPolygon();
    QPolygonF otherP  =  other->getPolygon();

    return thisP.intersects(otherP);
#else
    for (auto & ep1: std::as_const(*this))
    {
        QLineF l1 = ep1->getLine();
        for (auto & ep2 : std::as_const(*other))
        {
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

////////////////////////////////////////
///
/// Face Set
///
////////////////////////////////////////

void FaceSet::sortByPositon()
{
    QVector<FacePtr> newSet;

    for (auto & fp : std::as_const(*this))
    {
        if (newSet.isEmpty())
        {
            newSet.push_back(fp);
        }
        else
        {
            sortByPositon(fp,newSet);
        }
    }

    clear();

    QVector<FacePtr> & faces = *this;
    faces = newSet;
}

void FaceSet::sortByPositon(FacePtr fp, QVector<FacePtr> & newSet)
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

    for (auto & aface : std::as_const(*this))
    {
        deb << "F" << dcel->faceIndex(aface);
        if (aface->outer)
            deb << " OUTER\t";
        else
            deb << "      \t";

        deb << " size = " << aface->area << "\t";

        EdgePtr head = aface->incident_edge.lock();
        if (!head)
        {
            deb << "NULL edge";
        }
        else
        {
            EdgePtr e = head;
            do
            {
                dcel->print_edge(e,deb);
                e = e->next.lock();
            } while (e != head);
        }
        deb << endl;
    }
    qDebug().noquote() << astring;
}

void FaceGroups::select(int idx)
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

void FaceGroups::deselect()
{
    QVector<FaceSetPtr> & self = *this;
    for (int i=0; i < size(); i++)
    {
        FaceSetPtr fsp = self[i];
        fsp->selected = false;
    }
}

void FaceGroups::deselect(int idx)
{
    if (idx >= 0 && idx < size())
    {
        QVector<FaceSetPtr> & self = *this;
        FaceSetPtr fsp = self[idx];
        fsp->selected = false;
    }
}

bool FaceGroups::isSelected(int idx)
{
    if (idx >= 0 && idx < size())
    {
        QVector<FaceSetPtr> & self = *this;
        FaceSetPtr fsp = self[idx];
        return fsp->selected;
    }
    return false;
}

int FaceGroups::totalSize()
{
    int tot = 0;
    for (const auto & fset : std::as_const(*this))
    {
        tot += fset->size();
    }
    return tot;
}
