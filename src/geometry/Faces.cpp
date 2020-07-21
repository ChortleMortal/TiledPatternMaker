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
#include "geometry/faces.h"
#include "geometry/facecycles.h"
#include "geometry/point.h"
#include "geometry/loose.h"
#include "geometry/intersect.h"
#include "base/utilities.h"
#include <QPolygonF>

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

void Faces::buildFacesOriginal(MapPtr map)
{
    map->verifyMap("buildFaces");

    NeighbourMap & nmap = map->getNeighbourMap();

    clearFaces();

    for (auto edge : map->getEdges())
    {
        if( !v1.contains(edge) )
        {
            handleVertex(nmap, edge->getV1(), edge);
        }
        if( !v2.contains(edge) )
        {
            handleVertex(nmap, edge->getV2(), edge);
        }
    }

    qDebug() << "buildFacesOriginal:: all=" << allFaces.size() << "v1Edges" << v1.size() << "v2Edges" << v2.size();
}

// This algorithm doesn't distinguish between clockwise and
// counterclockwise.  So every map will produce one extraneous
// face, namely the countour that surrounds the whole map.
// By the nature of extractFace, the surrounding polygon will
// be the only clockwise polygon in the set.  So check for
// clockwise and throw it away.
void Faces::handleVertex(NeighbourMap & nmap, VertexPtr vert, EdgePtr edge)
{
    //qDebug() << "handleVertex:"  << "vert=" << vert->getTmpVertexIndex() << "edge=" << edge->getTmpEdgeIndex();

    FacePtr face = extractFace(nmap, vert, edge);
    Q_ASSERT(face->isValid());

    if (face->isClockwise())
    {
        allFaces.push_back(face);
    }

    for(auto edge : *face)
    {
        VertexPtr from = edge->getV1();
        VertexPtr to   = edge->getV2();

        NeighboursPtr np = nmap.getNeighbours(from);
        EdgePtr conn     = np->getNeighbour(to);

        if (!conn)
        {
            qWarning("no face to insert in map");
            continue;
        }
        if( conn->getV1() == from )
        {
            //qDebug() << "v1 insert edge" << conn->getTmpEdgeIndex();
            v1.insert(conn, face);
        }
        else
        {
            Q_ASSERT(conn->getV2() == from);
            //qDebug() << "v2 insert edge" << conn->getTmpEdgeIndex();
            v2.insert(conn, face);
        }
    }
}


// Walk from a vertex along an edge, always taking the current edge's neighbour at every vertex.
// The set of vertices encountered determines a face.
FacePtr Faces::extractFace(NeighbourMap & nmap, VertexPtr vert, EdgePtr edge)
{
    bool debugFaces = false;

    FacePtr face = make_shared<Face>();

    if (debugFaces) qDebug() << "extractFace: Edge" << edge->getTmpEdgeIndex() << "vertex" << vert->getTmpVertexIndex();

    EdgePtr   lastEdge;
    EdgePtr   lastPush;
    EdgePtr   pushEdge;

    VertexPtr from;
    VertexPtr to;

    EdgePtr currentEdge = edge;

    if (currentEdge->getV1() == vert)
    {
        pushEdge = currentEdge;
        from     = vert;
        to       = currentEdge->getV2();
    }
    else
    {
        Q_ASSERT(currentEdge->getV2() == vert);
        pushEdge = currentEdge->getSwappedEdge();
        from     = vert;
        to       = currentEdge->getV1();
    }

    while( true )
    {
        face->push_back(pushEdge);

        if (debugFaces) qDebug() << "added edge"  << pushEdge->getTmpEdgeIndex()
                                 << "from" << pushEdge->getV1()->getTmpVertexIndex()
                                 << "to"   << pushEdge->getV2()->getTmpVertexIndex();

        lastPush  = pushEdge;
        lastEdge  = currentEdge;

        if (debugFaces) qDebug() << "next vert:" << to->getTmpVertexIndex();
        NeighboursPtr np = nmap.getNeighbours(to);
        if (np->numNeighbours() < 2)
            return face;

        BeforeAndAfter ba = np->getBeforeAndAfter(currentEdge);
        currentEdge       = ba.before;
        if (debugFaces) qDebug() << "next edge:" << currentEdge->getTmpEdgeIndex();

        if (currentEdge->getV1() == lastPush->getV2())
        {
            pushEdge    = currentEdge;
        }
        else
        {
            pushEdge    = currentEdge->getSwappedEdge();
            if (debugFaces) qDebug() << "push edge is" << currentEdge->getTmpEdgeIndex() << "swapped";
        }

        from = to;
        if (from == currentEdge->getV2())
        {
            to = currentEdge->getV1();
        }
        else
        {
            Q_ASSERT(from == currentEdge->getV1());
            to = currentEdge->getV2();
        }

        if (currentEdge == edge)
        {
            return face;
        }
    }
}

void Faces::buildFacesNew23()
{
    //map->cleanse();

    for (auto face : allFaces)
    {
        face->sortForComparison();
    }

    removeDuplicates();

    decomposeCrossoverFaces();

    removeOverlaps();

    qDebug() << "Faces::extractFacesNew23 processing edges - done. Cleansed allFaces= " << allFaces.size();
    faceGroup.clear();
    zapFaceStates(FACE_UNDONE);

    // create face groups
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
    std::sort(faceGroup.begin(), faceGroup.end(), FaceSet::sort);    // largest first
    //dumpFaceGroup("algorithm 23 post sort");
    qDebug() << "Num face groups=" << faceGroup.size();
}


////////////////////////////////////////////////////
///
/// Assign Colors
///
////////////////////////////////////////////////////

// The main interface to this file.  Given a map and two
// empty vectors, this function fills the vectors with arrays
// of points corresponding to a two-colouring of the faces of the map.

void Faces::assignColorsOriginal(MapPtr map)
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
        assignColorsToFaces(map,facesToDo); // Propagate colours using a DFS (Depth First Search).
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
    qDebug() << "Faces::assignColorsNew2" << faceGroup.size() << faceGroup.totalSize();;

    // first make the color set size the same as the face group size
    if (colorSet.size() < faceGroup.size())
    {
        qWarning() <<  "faces=" << faceGroup.size() << "colors=" << colorSet.size();
        int diff = faceGroup.size() - colorSet.size();
        for (int i = 0; i < diff; i++)
        {
            TPColor tpc(Qt::yellow);
            tpc.hidden = true;
            colorSet.addColor(tpc);
        }
    }
    else if (colorSet.size() > faceGroup.size())
    {
        qWarning() <<  "faces=" << faceGroup.size() << "colors=" << colorSet.size();
        colorSet.resize(faceGroup.size());
    }
    Q_ASSERT(colorSet.size() == faceGroup.size());

   // assign the color set to the sorted group
   colorSet.resetIndex();
   for (auto face : faceGroup)
   {
       face->tpcolor = colorSet.getNextColor();
   }
}

void  Faces::assignColorsNew3(ColorGroup & colorGroup)
{
    qDebug() << "Faces::assignColorsNew3" << faceGroup.size() << faceGroup.totalSize();;

    // first make the color set size the same as the face group size
    if (colorGroup.size() < faceGroup.size())
    {
        qWarning() <<  "faces=" << faceGroup.size() << "colors=" << colorGroup.size();
        int diff = faceGroup.size() - colorGroup.size();
        for (int i = 0; i < diff; i++)
        {
            ColorSet cset;
            TPColor tpc(Qt::yellow);
            cset.addColor(tpc);
            cset.hide(true);
            colorGroup.addColorSet(cset);
        }
    }
    else if (colorGroup.size() > faceGroup.size())
    {
        qWarning() <<  "faces=" << faceGroup.size() << "colors=" << colorGroup.size();
        colorGroup.resize(faceGroup.size());
    }
    Q_ASSERT(colorGroup.size() == faceGroup.size());

   sortFaceSetsByPosition();   // this is  helpful for consistency

   // assign the color group to the sorted group
   colorGroup.resetIndex();
   for (auto it = faceGroup.begin(); it != faceGroup.end(); it++)
   {
       FaceSetPtr fsp = *it;
       fsp->colorSet  = colorGroup.getNextColorSet();
   }
}

// Propagate colours using a DFS (Depth First Search).
void Faces::assignColorsToFaces(MapPtr map, FaceSet & fset)
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

        for (auto edge : *face)
        {
            FacePtr nfi = getTwin(map, edge );
            if (!nfi)
            {
                if (debug & DEBUG_ASSIGN) qDebug().noquote() << "    no twin:";
                continue;
            }
            if (debug & DEBUG_ASSIGN) qDebug().noquote() << "  Twin face:" <<  Utils::addr(nfi.get()) << sFaceState[nfi->state];
            switch( nfi->state )
            {
            case FACE_UNDONE:
                nfi->state = FACE_PROCESSING;
                if (debug & DEBUG_ASSIGN) qDebug().noquote() << "    Pushing:" <<  Utils::addr(nfi.get()) << sFaceState[nfi->state];
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
FacePtr Faces::getTwin(MapPtr map, EdgePtr edge)
{
    const VertexPtr from = edge->getV1();
    const VertexPtr to   = edge->getV2();

    NeighbourMap & nmap = map->getNeighbourMap();
    NeighboursPtr np    = nmap.getNeighbours(from);
    EdgePtr conn     = np->getNeighbour(to);
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
    qDebug() << "Faces::removeOverlaps  allFaces=" << allFaces.size();
    zapFaceStates(FACE_UNDONE);
    int removed = 0;
    FaceSet overlaps;

    for (auto it = allFaces.begin(); it != allFaces.end(); it++)
    {
        FacePtr f1 = *it;

        if (f1->state == FACE_UNDONE)
            f1->state = FACE_DONE;

        //if (it+1 == allFaces.end())
        //    continue;
        for (auto it2 = it+1; it2 != allFaces.end(); it2++)
        {
            FacePtr f2 = *it2;

            if (f2->state != FACE_UNDONE)
                continue;

            //if (f1->overlaps(f2))
            if (isOverlapped(f1,f2))
            {
#if 1
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
#endif
            }
        }
    }
    //qDebug() << "to remove=" << removed << "out of" << allFaces.size();
    for (auto it = overlaps.begin(); it != overlaps.end(); it++)
    {
        FacePtr fp = *it;
        allFaces.removeAll(fp);
    }
    qDebug() << "removed overlaps - allfaces:" << allFaces.size();
}

void Faces::decomposeCrossoverFaces()
{
    qDebug() << "decomposeCrossoverFaces start:" << allFaces.size();
    FaceSet fset = allFaces;
    allFaces.clear();
    for (auto face : fset)
    {
        if (face->containsCrossover())
        {
            //qWarning() <<  "need to break into smaller faces << size=" << face->size();
            Q_ASSERT(face->isValid());
            FaceSet fset = face->decompose();
            //qDebug() << "decomposed faces=" << fset.size();
            for (auto face2 : fset)
            {
                face2->sortForComparison();
                allFaces.push_back(face2);
            }
            //qDebug() << "Face size now" << face->size();
            if (face->size())
            {
                face->sortForComparison();
                allFaces.push_back(face);
            }
        }
        else
        {
            allFaces.push_back(face);
        }
    }
    qDebug() << "decomposeCrossoverFaces end:" << allFaces.size();
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

bool Faces::isOverlapped(const FacePtr a, const FacePtr b)
{
    QPolygonF pa = a->getPolygon();
    QPolygonF pb = b->getPolygon();

    if (pa.intersects(pb))
    {
        QPolygonF p3 = pa.intersected(pb);
        if (!p3.isEmpty())
        {
            //qDebug() << "overlapping";
            return true;
        }
        else
        {
            //qDebug() << "touching";
            return  false;
        }
    }
    return  false;
}

void Faces::removeDuplicates()
{
    FaceSet qvfp;

    qDebug() << "Faces::Remove duplicates - start count="  << allFaces.size();
    for (auto face : allFaces)
    {
        if (face->getNumSides() == 0)
        {
            qDebug() << "empty face";
            continue;
        }
        bool found = false;
        for (auto face2 : qvfp)
        {
            if (face->equals(face2))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            qvfp.push_back(face);
        }
    }

    allFaces = qvfp; // replace

    qDebug() << "Faces::Remove duplicates - end   count="  << allFaces.size();
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
        qDebug().noquote() << i << Utils::addr(fip.get()) << "state=" << sFaceState[fip->state] << "sides=" << fip->size() << "area=" << fip->getArea();
        fip->dump();
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

QPolygonF Face::getPolygon()
{
    return getPoly();
}

bool  Face::overlaps(FacePtr other)
{
    QPolygonF thisP   =  getPolygon();
    QPolygonF otherP  =  other->getPolygon();

    return thisP.intersects(otherP);
}

QPolygonF Face::subtracted(FacePtr other)
{
    QPolygonF thisP   =  getPolygon();
    QPolygonF otherP  =  other->getPolygon();
    QPolygonF p       = thisP.subtracted(otherP);
    return p;
}

qreal Face::getArea()
{
    if (_areaCalculated)
    {
        return _area;
    }

    QPolygonF poly  = getPolygon();
    _area           = Utils::calcArea(poly);
    _areaCalculated = true;
    return _area;
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

QPointF Face::center()
{
    if (_center .isNull())
    {
        QPolygonF pts = getPolygon();
        _center = Point::center(pts);
    }
    return _center;
}

bool lessThanPoint(const QPointF &p1, const QPointF & p2)
{
    return ( p1.manhattanLength() < p2.manhattanLength());
 }

void Face::sortForComparison()
{
    sortedShadow = getPoly();
    QVector<QPointF> & vec = sortedShadow;
    std::sort(vec.begin(),vec.end(),lessThanPoint);
}

bool Face::containsCrossover()
{
    QVector<VertexPtr> qvec;
    for (auto edge : *this)
    {
        VertexPtr vp = edge->getV1();
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
    for (auto edge : *this)
    {
        VertexPtr vp = edge->getV1();
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
            for (auto edge : *this)
            {
                switch(state)
                {
                case NOT_STARTED:
                    if (edge->getV1() != vp)
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
                    if (edge->getV1() != vp)
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

void Face::dump()
{
    qDebug() << "=== FACE";
    EdgePoly & ep = *this;
    ep.dump();
    qDebug() << "=== END FACE";
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

int FaceGroup::totalSize()
{
    int tot = 0;
    for (auto fset : *this)
    {
        tot += fset->size();
    }
    return tot;
}
