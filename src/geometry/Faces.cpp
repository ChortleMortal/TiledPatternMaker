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

#include "base/configuration.h"
#include "geometry/Faces.h"
#include "geometry/facecycles.h"
#include "geometry/Point.h"
#include "geometry/Loose.h"
#include "geometry/Intersect.h"
#include "base/utilities.h"
#include <QPolygonF>

#define DELETE_CLOCKWISE
#undef  DELETE_LARGEST

#define E2STR(x) #x

static QString sFaceState[]
{
    E2STR(FACE_UNDONE),
    E2STR(FACE_PROCESSING),
    E2STR(FACE_DONE_BLACK),
    E2STR(FACE_DONE_WHITE),
    E2STR(FACE_DONE),
    E2STR(FACE_REMOVE)
};

////////////////////////////////////////
///
/// Faces
///
////////////////////////////////////////

Faces::Faces()
{
    Configuration * config = Configuration::getInstance();
    config->faceSet = &allFaces;
    config->selectedFace.reset();
}

void Faces::clearFaces()
{
    v1.clear();
    v2.clear();
    whiteFaces.clear();
    blackFaces.clear();
    faceGroup.clear();
    allFaces.clear();
}

////////////////////////////////////////////////////
///
/// Build Faces
///
////////////////////////////////////////////////////

void Faces::buildFacesOriginal(constMapPtr map)
{
    map->verify("buildFaces",false,true);

    clearFaces();
    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        EdgePtr edge = *e;
        if( !v1.contains(edge) )
        {
            handleVertex(edge->getV1(), edge);
        }
        if( !v2.contains(edge) )
        {
            handleVertex(edge->getV2(), edge);
        }
    }

    qDebug() << "buildFaces:: num=" << allFaces.size();
}

void Faces::buildFacesNew23(MapPtr map)
{
    map->cleanse();

    // First, build all the faces.
#if 1
    qDebug() << "Faces::extractFacesNew23 processing" << map->getEdges()->size() << "edges";
    FacePtr face;
    //int i = 0;
    for(auto e = map->getEdges()->begin(); e != map->getEdges()->end(); e++)
    {
        //qDebug() << "processing" << ++i << "out of"  << sz;
        EdgePtr edge = *e;
        if( !v1.contains(edge) )
        {
            face = extractFace(edge->getV1(),edge);
            v1.insert(edge,face);
            if (!isClockwise(face))
            {
                face->sortForComparion();
                allFaces.push_back(face);
            }
        }
        if( !v2.contains(edge) )
        {
           face = extractFace(edge->getV2(),edge);
           v2.insert(edge,face);
           if (!isClockwise(face))
           {
               face->sortForComparion();
               allFaces.push_back(face);
           }
        }
    }


    qDebug() << "Faces::extractFacesNew23 processing edges - done";
#else
    // new code
    FaceCycles fc;
    faces = fc.getFaceSet(map);
#endif


#ifdef DELETE_LARGEST
    removeLargest();
#endif

    removeDuplicates();

    //removeOverlaps();

    // create face groups
    faceGroup.clear();
    zapFaceStates(FACE_UNDONE);
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        if (fp->state != FACE_UNDONE)
        {
            continue;
        }

        fp->state      = FACE_DONE;

        FaceSetPtr fsp = make_shared<FaceSet>();
        fsp->area      = fp->getArea();
        fsp->sides     = fp->getNumSides();
        fsp->push_back(fp);
        faceGroup.push_back(fsp);

        for (auto it2 = (it + 1); it2 != allFaces.end(); it2++)
        {
            FacePtr fp2 = *it2;
            if (fp2->state == FACE_UNDONE)
            {
                if (Loose::equals(fsp->area,fp2->getArea()) && (fsp->sides == fp2->getNumSides()))
                {
                    fp2->state = FACE_DONE;
                    fsp->push_back(fp2);
                }
            }
        }
    }

   //dumpFaceGroup("algorithm 23");
   std::sort(faceGroup.begin(), faceGroup.end(), FaceSet::sortByArea);    // largest first
   //dumpFaceGroup("algorithm 23 post sort");
}

////////////////////////////////////////////////////
///
/// Assign Colors
///
////////////////////////////////////////////////////

// The main interface to this file.  Given a map and two
// empty vectors, this function fills the vectors with arrays
// of points corresponding to a two-colouring of the faces of the map.

void Faces::assignColorsOriginal()
{
    whiteFaces.clear();
    blackFaces.clear();

    // recurse through faces to assign colours
    // this now deals with non-contiguous figures
    FaceSet facesToDo = allFaces;      // copy
    int facesToProcess;
    int facesLeft;
    do
    {
        facesToProcess = facesToDo.size();
        qDebug() << "facesToProcess=" << facesToProcess;
        assignColorsToFaces(facesToDo); // Propagate colours using a DFS (Depth First Search).
        addFaceResults(facesToDo);
        facesLeft = facesToDo.size();
        qDebug() << "facesLeft=" << facesLeft;
    } while ((facesLeft != 0) &&  (facesLeft < facesToProcess));

    qDebug() << "done: facesLeft =" << facesLeft << "facesProcessed =" << facesToProcess;
    qDebug() << "done: white count =" << whiteFaces.size() << "black count =" << blackFaces.size();
}

// DAC Notes
// An edge can only be in two faces
// A vertex can be in multiple faces

void Faces::assignColorsNew1()
{
    //debugListFaces("BEFORE ASSIGN");

    eFaceState newState = FACE_DONE_WHITE;     // seed
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        if (fp->state != FACE_UNDONE)
        {
            continue;
        }
        fp->state = newState;
        qreal size = fp->getArea();
        for (auto it2 = allFaces.begin(); it2 != allFaces.end(); it2++)
        {
            FacePtr fp2 = *it2;
            if (fp2->state == FACE_UNDONE)
            {
                if (Loose::equals(size,fp2->getArea()))
                {
                    fp2->state = newState;
                }
            }
        }
        newState = (newState == FACE_DONE_WHITE) ? FACE_DONE_BLACK : FACE_DONE_WHITE;
    }

   //debugListFaces("AFTER ASSIGN");

    for( int idx = 0; idx < allFaces.size(); ++idx )
    {
        FacePtr fi = allFaces.at( idx );
        if( fi->state == FACE_DONE_WHITE )
        {
            whiteFaces.push_back(fi);
        }
        else if( fi->state == FACE_DONE_BLACK )
        {
            blackFaces.push_back(fi);
        }
        else
        {
            Q_ASSERT(false);
        }
    }

    whiteFaces.sortByPositon();
    blackFaces.sortByPositon();
}


void Faces::assignColorsNew2(ColorSet & colorSet)
 {
   // assign the color set to the sorted group
   colorSet.resetIndex();
   for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
   {
       FaceSetPtr fsp = *it;
       fsp->tpcolor     = colorSet.getNextColor();
   }

   //dumpFaceGroup("algorithm 2 post sort");
}

void  Faces::assignColorsNew3(ColorGroup & colorGroup)
{
   sortFaceSetsByPosition();

   // assign the color group to the sorted group
   colorGroup.resetIndex();
   for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
   {
       FaceSetPtr fsp = *it;
       fsp->colorSet  = colorGroup.getNextColorSet();
   }
}

// Propagate colours using a DFS (Depth First Search).
void Faces::assignColorsToFaces(FaceSet & fset)
{
#define DEBUG_FACES     1
#define DEBUG_EDGE_MAP  2
#define DEBUG_ASSIGN    4

    static int debug = 0; //DEBUG_FACES | DEBUG_EDGE_MAP | DEBUG_ASSIGN;

    if (fset.size() == 0)
    {
        qWarning() << "no faces to assign";
        return;
    }

    if (debug & DEBUG_FACES)dumpAllFaces("allfaces BEFORE ASSIGN");
    if (debug & DEBUG_EDGE_MAP) dumpEdgeFaceMap(v1,"v1");
    if (debug & DEBUG_EDGE_MAP) dumpEdgeFaceMap(v2,"v2");

    QStack<FacePtr> st;
    FacePtr face = fset.at(0);
    face->state = FACE_PROCESSING;
    st.push(face );

    int pops = 0;
    int pushes = 1;
    while( !st.empty() )
    {
        face = st.pop();
        pops++;
        if (debug & DEBUG_ASSIGN) qDebug().noquote() << "Popped face:" << Utils::addr(face.get()) << sFaceState[face->state];

        eColor color = C_WHITE;     // seed

        for( int idx = 0; idx < face->size(); ++idx )
        {
            FacePtr nfi = getTwin(face, idx);
            if (!nfi)
            {
                if (debug & DEBUG_ASSIGN) qDebug().noquote() << "    no twin:"  << idx;
                continue;
            }
            if (debug & DEBUG_ASSIGN) qDebug().noquote() << "  Twin face:" << idx << Utils::addr(nfi.get()) << sFaceState[nfi->state];
            switch( nfi->state )
            {
            case FACE_UNDONE:
                nfi->state = FACE_PROCESSING;
                if (debug & DEBUG_ASSIGN) qDebug().noquote() << "    Pushing:" << idx << Utils::addr(nfi.get()) << sFaceState[nfi->state];
                st.push( nfi );
                pushes++;
                break;

            case FACE_PROCESSING:
            case FACE_DONE:
            case FACE_REMOVE:
                break;

            case FACE_DONE_BLACK:
                if( color == C_WHITE )
                {
                    qWarning("Filling problem 1");
                }
                color = C_BLACK;
                break;

            case FACE_DONE_WHITE:
                if( color == C_BLACK )
                {
                    qWarning("Filling problem 2");
                }
                color = C_WHITE;
                break;
            }
        }

        if (color == C_BLACK)
        {
            face->state = FACE_DONE_WHITE;
        }
        else
        {
            Q_ASSERT(color == C_WHITE);
            face->state = FACE_DONE_BLACK;
        }

        if (debug & DEBUG_ASSIGN) qDebug().noquote() << " Popped now:" << Utils::addr(face.get()) << sFaceState[face->state];
    }

    qDebug() << "Pushes=" << pushes << "Pops=" << pops;

    if (debug) dumpAllFaces("allfaces AFTER ASSIGN");
}

void Faces::handleVertex(VertexPtr vert, EdgePtr edge)
{
    FacePtr face = extractFace(vert, edge);
    //qDebug() << "Face" << Utils::addr(face.get()) << "sides=" << face->size();

    if( face->size() == 0)
    {
        return;
    }

#ifdef DELETE_CLOCKWISE
    if (isClockwise(face))
    {
        // This algorithm doesn't distinguish between clockwise and
        // counterclockwise.  So every map will produce one extraneous
        // face, namely the countour that surrounds the whole map.
        // By the nature of extractFace, the surrounding polygon will
        // be the only clockwise polygon in the set.  So check for
        // clockwise and throw it away.
        qDebug() << "dumping clockwise face";
        return;
    }
#endif

    face->sortForComparion();
    allFaces.push_back(face);

    for( int v = 0; v < face->size(); ++v )
    {
        VertexPtr from = face->at(v);
        VertexPtr to   = face->at((v+1) % face->size());
        EdgePtr conn   = from->getNeighbour(to);

        if (!conn)
        {
            qWarning("no face to insert in map");
            continue;
        }
        if( conn->getV1() == from )
        {
            v1.insert(conn, face);
        }
        else
        {
            Q_ASSERT(conn->getV2() == from);
            v2.insert(conn, face);
        }
    }
}

// Walk from a vertex along an edge, always taking the current edge's neighbour at every vertex.
// The set of vertices encountered determines a face.
FacePtr Faces::extractFace(VertexPtr from, EdgePtr edge)
{
    FacePtr ret   = make_shared<Face>();

    VertexPtr cur = from;
    EdgePtr  ecur = edge;

    while( true )
    {
        ret->push_back(cur);
        VertexPtr n = ecur->getOtherV(cur->getPosition());

        if( n->numNeighbours() < 2 )
        {
            return ret;
        }

        BeforeAndAfter ba = n->getBeforeAndAfter(ecur);
        ecur = ba.before;

        cur = n;
        if (cur == from)
        {
            return ret;
        }
    }
}

void Faces::addFaceResults(FaceSet & fset)
{
    FaceSet  faces2do;
    for( int idx = 0; idx < fset.size(); ++idx )
    {
        FacePtr fi = fset.at( idx );
        if( fi->state == FACE_DONE_WHITE )
        {
            whiteFaces.push_back(fi);
        }
        else if( fi->state == FACE_DONE_BLACK )
        {
            blackFaces.push_back(fi);
        }
        else
        {
            fi->state = FACE_UNDONE;
            faces2do.push_back(fi);
        }
    }
    fset = faces2do;
}

// Find the face_info that lies across the given edge.
// Used to propagate the search to adjacent faces, giving them opposite colours.
FacePtr Faces::getTwin(FacePtr fi, int idx)
{
    const VertexPtr from = fi->at(idx);
    const VertexPtr to   = fi->at((idx+1) % fi->size());
    EdgePtr conn         = from->getNeighbour(to);
    if (!conn)
    {
        return FacePtr();
    }
    if (conn->getV1() == from)
    {
        //Q_ASSERT(v2.contains(conn));
        return v2.value(conn);
    }
    else
    {
        Q_ASSERT(conn->getV2() == from);
        //Q_ASSERT(v1.contains(conn));
        return v1.value(conn);
    }
}

// Is a polygon (given here as a vector of Vertex instances) clockwise?
// We can answer that question by finding a spot on the polygon where
// we can distinguish inside from outside and looking at the edges
// at that spot.  In this case, we look at the vertex with the
// maximum X value.  Whether the polygon is clockwise depends on whether
// the edge leaving that vertex is to the left or right of the edge
// entering the vertex.  Left-or-right is computed using the sign of the cross product.
bool Faces::isClockwise(FacePtr face)
{
    int sz = face->size();

    // First, find the vertex with the greatest X coordinate.

    int imax = 0;
    qreal xmax = face->at( 0 )->getPosition().x();

    for( int idx = 1; idx < sz; ++idx )
    {
        qreal x = face->at( idx )->getPosition().x();
        if( x > xmax )
        {
            imax = idx;
            xmax = x;
        }
    }

    QPointF pmax  = face->at(imax)->getPosition();
    QPointF pnext = face->at((imax+1) % sz)->getPosition();
    QPointF pprev = face->at((imax+sz-1) %sz)->getPosition();

    QPointF dprev = pmax -  pprev;
    QPointF dnext = pnext - pmax;

    return Point::cross(dprev, dnext ) <= 0.0;
}

// has same purpose as isClockwise()
void  Faces::removeLargest()
{
    qDebug() << "Faces::removeLargest";
#if 0
    // remove single face with largest number of sides
    FacePtr largest;
    int largestSides = 0;
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        int sides = fp->getNumSides();
        if (sides > largestSides)
        {
            largestSides = sides;
            largest      = fp;
        }
    }
    allFaces.removeAll(largest);

#else
    // remove faces with the same largest area
    QVector<FacePtr> toRemove;
    qreal maxSize = 0.0;
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        qreal area = fp->getArea();
        if (area > maxSize)
        {
            maxSize = area;
        }
    }
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        if (Loose::equals(maxSize,fp->getArea()))
        {
            toRemove.push_front(fp);
        }
    }
    for (auto it = toRemove.begin(); it != toRemove.end(); it++)
    {
        FacePtr fp = *it;
        allFaces.removeAll(fp);
    }
#endif
}

void Faces::removeOverlaps()
{
    qDebug() << "Faces::removeOverlaps";
    zapFaceStates(FACE_UNDONE);
    int removed = 0;
    QVector<FacePtr> overlaps;
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr f1 = *it;

        if (f1->state == FACE_UNDONE)
            f1->state = FACE_DONE;

        if (it+1 == allFaces.end())
            continue;

        for (auto it2 = it+1; it2 != allFaces.end(); it2++)
        {
            FacePtr f2 = *it2;

            if (f2->state != FACE_UNDONE)
                continue;

            //if (f1->overlaps(f2))
            if (isOverlapped(f1,f2))
            {
                // which one is the larger overlapper?
                if (f2->getArea() >= f1->getArea())
                {
                    if (f2->state != FACE_REMOVE)
                    {
                        f2->state = FACE_REMOVE;
                        removed++;
                        overlaps.push_back(f2);
                    }
                }
                else
                {
                    if (f1->state != FACE_REMOVE)
                    {
                        f1->state = FACE_REMOVE;
                        removed++;
                        overlaps.push_back(f1);
                    }
                }
            }
        }
    }
    //qDebug() << "to remove=" << removed << "out of" << allFaces.size();
    for (auto it = overlaps.begin(); it != overlaps.end(); it++)
    {
        FacePtr fp = *it;
        allFaces.removeAll(fp);
    }
}

void  Faces::sortFaceSetsByPosition()
{
    for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
    {
        FaceSetPtr fsp = *it;
        fsp->sortByPositon();
    }
}

void Faces::zapFaceStates(eFaceState state)
{
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        fp->state = state;
    }
}

qreal Faces:: determinant(QPointF vec1, QPointF vec2)
{
    return vec1.x() * vec2.y() - vec1.y() * vec2.x();
}

//one edge is a-b, the other is c-d
bool Faces::edgeIntersection(QPointF a, QPointF b, QPointF c, QPointF d)
{
    double det = determinant(b - a, c - d);
    double t   = determinant(c - a, c - d) / det;
    double u   = determinant(b - a, c - a) / det;
    if ((t < 0) || (u < 0) || (t > 1) || (u > 1))
    {
        //return NO_INTERSECTION;
        return false;
    }
    else
    {
        //return a * (1 - t) + t * b;
        return true;
    }
}

bool Faces::isOverlapped(FacePtr a, FacePtr b)
{
    QPointF p1,p2,p3,p4;
    int asize = a->size();
    int bsize = b->size();
    for (int i = 0; i < asize; i++)
    {
        p1 = a->at(i)->getPosition();
        p2 = a->at((i+1)%asize)->getPosition();
        for (int j=0; j < bsize; j++)
        {
            p3 = b->at(j)->getPosition();
            p4 = b->at((j+1)%bsize)->getPosition();

            if (edgeIntersection(p1,p2,p3,p4))
            {
                if (!Intersect::getNearIntersection(p1,p2,p3,p4).isNull())
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void Faces::removeDuplicates()
{
    FaceSet qvfp;

    qDebug() << "Faces::Remove duplicates - start count="  << allFaces.size();
    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr fp = *it;
        if (fp->getNumSides() == 0)
        {
            qDebug() << "empty face";
            continue;
        }
        bool found = false;
        for (auto it2 = qvfp.begin(); it2 != qvfp.end(); it2++)
        {
            FacePtr fp2 = *it2;
            if (fp->equals(fp2))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            qvfp.push_back(fp);
        }
    }

    allFaces = qvfp; // replace

    qDebug() << "Remove duplicates - end   count="  << allFaces.size();
}


////////////////////////////////////////
///
/// Faces Debug
///
////////////////////////////////////////

void Faces::dumpFaceGroup(QString title)
{
    qDebug().noquote() << "=============== FACE SET" << title << "=================================";
    for (int i=0; i < faceGroup.size(); i++)
    {
        FaceSetPtr fsp = faceGroup[i];
        qDebug().noquote() << i << "area=" << fsp->area <<"sides=" << fsp->sides << "count=" << fsp->size() << "color" << fsp->colorSet.colorsString();
    }
    qDebug().noquote() << "===============END" << title << "=============================";
}

void Faces::dumpAllFaces(QString title)
{
    qDebug().noquote() << "=============== FACES" << title << "=================================";
    for (int i=0; i < allFaces.size(); i++)
    {
        FacePtr fip = allFaces[i];
        qDebug().noquote() << i << Utils::addr(fip.get()) << "state=" << sFaceState[fip->state] << "sides=" << fip->size() << "area=" << fip->getArea()  << fip->dump();
    }
    qDebug().noquote() << "===============END" << title << "=============================";
}

void Faces::dumpEdgeFaceMap(QMap<EdgePtr,FacePtr> & dmap, QString title)
{
    qDebug().noquote() << "=============== MAP" << title << "=================================";
    for (auto it = dmap.constBegin(); it != dmap.constEnd(); it++)
    {
        qDebug() << "Edge:"  << Utils::addr(it.key().get()) <<  "Face:" << Utils::addr(it.value().get()) << "face sides=" << it.value()->size();
    }
    qDebug().noquote() << "=============== END" << title << "=================================";

}

void  Faces::purifyMap(MapPtr map)
{
    map->cleanse();
    map->calcVertexEdgeCounts();
    map->removeVerticesWithEdgeCount(1);
    map->calcVertexEdgeCounts();
}

void Faces::purifyFaces()
{
    qWarning() << "Faces::purifyFaces - currently does nothing useful";
    int count = 0;

    for (int i=0; i < allFaces.size(); i++)
    {
        FacePtr fp = allFaces[i];
        if (fp->isClosed())
        {
            qDebug() << "face is closed";
        }
        if (i == 55)
        {
            qDebug() << "bingo";
        }
        bool rv = fp->containsCrossover();
        if (rv)
            count++;
    }
    qDebug() << count << "crossovers out of" << allFaces.size();
}

////////////////////////////////////////
///
/// Face
///
////////////////////////////////////////

Face::Face()
{
    state = FACE_UNDONE;
    _area = 0.0;
    _areaCalculated = false;
}

PolyPtr Face::getPolygon()
{
    PolyPtr pts = make_shared<QPolygonF>();
    for( int i = 0; i < size(); i++ )
    {
        *pts << at(i)->getPosition();
    }
    return pts;
}

void Face::setPolygon(PolyPtr poly)
{
    for (int i=0; i < size(); i++)
    {
        VertexPtr p = at(i);
        p->setPosition(poly->at(i));
    }
}

bool  Face::overlaps(FacePtr other)
{
    QPolygonF thisP   =  *getPolygon();
    QPolygonF otherP  =  *other->getPolygon();

    return thisP.intersects(otherP);
}

PolyPtr Face::subtracted(FacePtr other)
{
    PolyPtr thisP   =  getPolygon();
    PolyPtr otherP  =  other->getPolygon();
    QPolygonF p     = thisP->subtracted(*otherP);
    return make_shared<QPolygonF>(p);
}

qreal Face::getArea()
{
    if (_areaCalculated)
    {
        return _area;
    }

    QPolygonF poly  = *getPolygon();
    _area           = Utils::calcArea(poly);
    _areaCalculated = true;
    return _area;
}

bool Face::equals(FacePtr other)
{
    return getSorted() == other->getSorted();
}

QVector<VertexPtr> & Face::getSorted()
{
    Q_ASSERT(sortedShadow.size() != 0);
    return sortedShadow;
}

QPointF Face::center()
{
    if (_center .isNull())
    {
        _center = Point::center(*getPolygon());
    }
    return _center;
}

void Face::sortForComparion()
{
    sortedShadow.clear();
    QVector<VertexPtr> & self = *this;
    for (auto it = self.begin(); it !=self.end(); it ++)
    {
        VertexPtr v = *it;
        bool found = false;
        for (int i = 0; i < sortedShadow.size(); i++)
        {
            VertexPtr vert = sortedShadow[i];
            if (v.get() <= vert.get())
            {
                sortedShadow.insert(i,v);
                found = true;
                break;
            }
        }
        if (!found)
        {
            sortedShadow.push_back(v);
        }
    }
}

bool  Face::containsCrossover()
{
    QVector<VertexPtr> qvec;
    for (auto it= begin(); it != end(); it++)
    {
        VertexPtr vp = *it;
        if (!qvec.contains(vp))
        {
            qvec.push_back(vp);
        }
        else
        {
            if (vp != last())
            {
                qDebug() << "crossover detected";
                return true;
            }
        }
    }
    return false;
}

QString Face::dump()
{
    QString astring;
    QDebug  deb(&astring);

    QVector<VertexPtr> & vertices = *this;
    for (auto it = vertices.begin(); it != vertices.end(); it++)
    {
        VertexPtr v = *it;
        QPointF pt = v->getPosition();
        deb << "[" << Utils::addr(v.get()) << pt << "]";
    }
    astring.remove("QPointF");
    return astring;
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
    for (auto it = newSet.begin(); it != newSet.end(); it++)
    {
        // sort by y-axis
        FacePtr fp2 = *it;
        QPointF pt2 = fp2->center();
        if (pt.y() < pt2.y())
        {
            newSet.insert(it,fp);   // insert before
            return;
        }
        else if (Loose::equals(pt.y(), pt2.y()) )
        {
            // sort by x-axis
            while (it != newSet.end())
            {
                fp2 = *it;
                pt2 = fp2->center();
                if (!Loose::equals(pt.y(), pt2.y()))
                {
                    newSet.insert(it,fp);
                    return;
                }
                if (pt.x() < pt2.x())
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

