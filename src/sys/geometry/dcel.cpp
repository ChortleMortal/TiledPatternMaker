#include <QDebug>
#include <QElapsedTimer>
#include <QApplication>
#include <math.h>

#include "sys/geometry/dcel.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/vertex.h"
#include "sys/geometry/neighbours.h"
#include "sys/qt/tpm_io.h"


using namespace std;

#define INF 10000

int DCEL::refs  = 0;

////////////////////////////////////////////////////////////////////////////////////////////////
///
///   DCEL  - doubly connected edge list
///
////////////////////////////////////////////////////////////////////////////////////////////////

DCEL::DCEL(Map * sourcemap) : Map("DCEL")
{
    qDebug() << "Creating DCEL";

    QElapsedTimer timer;
    timer.start();

    this->sourcemap = sourcemap;

    vertices    = sourcemap->vertices;
    edges       = sourcemap->edges;
    qDebug().noquote() << "DECL: importing" << sourcemap->info();

    qDebug().noquote() << DCEL::info();

    cleanseEdges();
    qDebug().noquote() << DCEL::info();

    buildDCEL();
    qDebug().noquote() << DCEL::info();

    qreal time    = timer.elapsed();
    double qdelta = time /1000.0;
    QString sDelta = QString("%1").arg(qdelta, 8, 'f', 3);

    qDebug().noquote() << "DCEL: complete time:" << sDelta << DCEL::info();
    refs++;
}

DCEL::~DCEL()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "Deleting DCEL";
    clean();
#endif
    refs--;
}

void DCEL::cleanseEdges()
{
    qDebug() << __FUNCTION__;

    QVector<EdgePtr> forDeletion;
    // edges which have an unconnected end or ends make no sense  for building faces
    for (const EdgePtr & edge : std::as_const(edges))
    {
        int n = sourcemap->getNeighbours(edge->v1)->numNeighbours();
        if (n <2 )
        {
            forDeletion.push_back(edge);
            continue;
        }
        n = sourcemap->getNeighbours(edge->v2)->numNeighbours();
        if (n <2 )
        {
            forDeletion.push_back(edge);
            continue;
        }
    }

    if (forDeletion.count())
    {
        qInfo() << "DCEL::cleanseEdges - removing" << forDeletion.count() << "edges";
    }

    for (EdgePtr & edge : forDeletion)
    {
        edges.removeOne(edge);
    }
}

void DCEL::buildDCEL()
{
    qDebug() << __FUNCTION__;

    // fill half edges
    QVector<EdgePtr> additions(edges.size());
    int i=0;
    for (const auto & e : std::as_const(edges))
    {
        // the map may be used to create many dcels
        e->dvisited = false;
        e->twin.reset();
        e->next.reset();
        e->incident_face.reset();

        VertexPtr v1 = e->v1;
        VertexPtr v2 = e->v2;
        v1->adjacent_vertices.push_back(v2);
        v2->adjacent_vertices.push_back(v1);

        EdgePtr e2 = e->createTwin();
        e2->twin   = e;
        e->twin    = e2;

        additions[i++] = e2;
    }

    edges.append(additions);

   
    fill_half_edge_table();

    fill_half_edge_faces();

    fill_face_table_inner_components();

    qDebug() << "DCEL: loaded";
}

void DCEL::fill_half_edge_table()
{
    qDebug() << __FUNCTION__ << summary();

    QVector<EdgePtr> deletions;
    for (const auto & edge : std::as_const(edges))
    {
        if (!edge->dvisited)
        {
            edge->dvisited = true;

            EdgePtr current = edge;
            EdgePtr next    = next_half_edge(current);
            while (next != edge)
            {
                if (next)
                {
                    next->dvisited  = true;
                    current->next   = next;
                    current         = next;
                    next            = next_half_edge(current);
                }
                else
                {
                    deletions.push_back(current);
                    break;
                }
            }
            current->next = edge;   // same as next
        }
    }

    qDebug().noquote() << "DCEL:" << summary() << "deletions=" <<  deletions.size();
    for (auto &  edge2 : std::as_const(deletions))
    {
        EdgePtr edget = edge2->twin.lock();
        edges.removeAll(edge2);
        edges.removeAll(edget);
    }

    qDebug().noquote() << "DCEL:" << summary();
}

EdgePtr DCEL::next_half_edge(const EdgePtr & edge)
{
    qreal max_angle = 0.0;

    VertexPtr v1    = edge->v1;
    VertexPtr v2    = edge->v2;
    QPointF p1      = v1->pt;
    QPointF p2      = v2->pt;

    VertexPtr next;
    for (const auto & weakv : std::as_const(v2->adjacent_vertices))
    {
        VertexPtr v = weakv.lock();
        if (v == v1)
        {
            continue;
        }
        else
        {
            qreal temp_angle = angle(p1, p2, v->pt);
            if(max_angle < temp_angle)
            {
                max_angle = temp_angle;
                next = v;
            }
        }
    }
    if (next)
    {
        return findEdge(v2,next,true);
    }
    else
    {
        EdgePtr dp;
        return dp;     // edge is not connected
    }
}

void DCEL::fill_half_edge_faces()
{
    qDebug() << __FUNCTION__;

    for (const auto & e : std::as_const(edges))
    {
        e->dvisited = false;
    }

    for (const auto & edge : std::as_const(edges))
    {
        if (edge->dvisited)
        {
            continue;
        }

        if (!edge->next.lock())
        {
            edge->dvisited = true;
            continue;
        }
        createFace(edge);
    }

    faces.sortByPositon();
    qDebug() << "DCEL: faces=" << faces.count();
}

void DCEL::createFace(const EdgePtr & head)
{
    double signedArea = 0;

    FacePtr aface = make_shared<Face>();
    faces.push_back(aface);

    EdgePtr oedge = head;

    double x1 = 0;
    double y1 = 0;
    do
    {
        //cout << "adding edge " << edgeIndex(oedge) << endl;
        //print_edge(oedge);
        oedge->incident_face = aface;
        oedge->dvisited = true;
        aface->push_back(oedge);

        double x1 = oedge->v1->pt.x();
        double y1 = oedge->v1->pt.y();
        double x2 = oedge->v2->pt.x();
        double y2 = oedge->v2->pt.y();
        signedArea += (x1 * y2 - x2 * y1);

        oedge = oedge->next.lock();
    }
    while (oedge != head);

    double x2 = head->v1->pt.x();
    double y2 = head->v1->pt.y();
    signedArea += (x1 * y2 - x2 * y1);

    aface->area          = abs(signedArea / 2);
    aface->incident_edge = head;

    if (signedArea > 0) // Assuming no standalone edge
        aface->outer = false;
    else
        aface->outer = true;
}

void DCEL::fill_face_table_inner_components()
{
    qDebug() << __FUNCTION__;

    for (const auto & hedge : std::as_const(edges))
    {
        EdgePtr edge = hedge;
        if (edge->incident_face.lock())
            continue;

        EdgePtr head  = edge;

        QVector<VertexPtr> verts;
        verts.push_back(edge->v1);
        edge = edge->next.lock();
        while (edge != head)
        {
            verts.push_back(edge->v1);
            edge = edge->next.lock();
        }

        FacePtr aface = check_if_inside(verts);
        if (!aface)
        {
            aface = findOuterFace();
        }

        aface->incident_edge = edge;

        edge->incident_face = aface;
        edge = edge->next.lock();
        while (edge != head)
        {
            edge->incident_face = aface;
            edge = edge->next.lock();
        }
    }
}

bool DCEL::check_if_point_is_inside(const VertexPtr & ver, const QVector<VertexPtr> & key)
{
    const int n = key.size();
    QPolygonF polygon1(n);
    for(int i = 0 ; i < n ; i++)
    {
        polygon1.push_back(key[i]->pt);
    }
    return isInside(polygon1, ver->pt);
}

FacePtr DCEL::check_if_inside(const QVector<VertexPtr> & verts)
{
    double   min_area    = 100021.1;
    double   self_area   = area_poly(verts);

    FacePtr insideFace;

    for (auto & aface : std::as_const(faces))
    {
        EdgePtr edge = aface->incident_edge.lock();
        if (!edge)
            edge = edges[0];       // king  of kludges
        EdgePtr head = edge;

        QVector<VertexPtr> key2;
        while(1)
        {
            key2.push_back(edge->v1);
            edge = edge->next.lock();
            if (edge == head)
                break;
        }

        bool flag = 1;
        for(int k = 0 ; k < verts.size() ; k++)
        {
            flag = flag & check_if_point_is_inside(verts[k] , key2);
            if(flag == 0)
                break;
        }
        if(flag)
        {
            double a = area_poly(key2);
            if(min_area > a && self_area != a && self_area < a)
            {
                min_area = a;
                insideFace = aface;
            }
        }
    }
    return insideFace;
}

FacePtr DCEL::findOuterFace()
{
    for (const auto & face : std::as_const(faces))
    {
        if (face->outer)
        {
            return face;
        }
    }
    FacePtr fp;
    return fp;
}

double DCEL::area_poly(const QVector<VertexPtr> &key)
{
    double x2, y2;
    double signedArea = 0;
    int l = key.size();
    for (int i = 0; i < l; i++)
    {
        double x1 = key[i]->pt.x();
        double y1 = key[i]->pt.y();
        if (i == l - 1)
        {
            x2 = key[0]->pt.x();
            y2 = key[0]->pt.y();
            signedArea += (x1 * y2 - x2 * y1);
        }
        else
        {
            x2 = key[i + 1]->pt.x();
            y2 = key[i + 1]->pt.y();
            signedArea += (x1 * y2 - x2 * y1);
        }
    }
    return abs(signedArea / 2);
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool DCEL::onSegment(const QPointF &p, const QPointF &q, const QPointF &r)
{
    if (q.x() <= max(p.x(), r.x()) && q.x() >= min(p.x(), r.x()) &&
        q.y() <= max(p.y(), r.y()) && q.y() >= min(p.y(), r.y()))
        return true;
    return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int DCEL::orientation(const QPointF & p, const QPointF & q, const QPointF & r)
{
    double val = (q.y() - p.y()) * (r.x() - q.x()) -
                 (q.x() - p.x()) * (r.y() - q.y());

    if (val == 0) return 0; // colinear
    return (val > 0) ? 1 : 2; // clock or counterclock wise
}

// The function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool DCEL::doIntersect(const QPointF & p1, const QPointF & q1, const QPointF & p2, const QPointF & q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;

    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;

    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;

    // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;

    return false; // Doesn't fall in any of the above cases
}

// Returns true if the point p lies inside the polygon[] with n vertices
bool DCEL::isInside(const QPolygonF &polygon, const QPointF &p)
{
    int sz = polygon.size();
    // There must be at least 3 vertices in polygon[]

    if (sz < 3) return false;

    // Create a point for line segment from p to infinite
    QPointF extreme = { INF, p.y() };

    // Count intersections of the above line with sides of polygon
    int count = 0, i = 0;
    do
    {
        int next = (i + 1) % sz;

        // Check if the line segment from 'p' to 'extreme' intersects
        // with the line segment from 'polygon[i]' to 'polygon[next]'
        if (doIntersect(polygon[i], polygon[next], p, extreme))
        {
            // If the point 'p' is colinear with line segment 'i-next',
            // then check if it lies on segment. If it lies, return true,
            // otherwise false
            if (orientation(polygon[i], p, polygon[next]) == 0)
                return onSegment(polygon[i], p, polygon[next]);

            count++;
        }
        i = next;
    } while (i != 0);

    // Return true if count is odd, false otherwise
    return count & 1; // Same as (count%2 == 1)
}

double DCEL::angle(const QPointF &p1, const QPointF &p2, const QPointF &p3)
{
    double x1 = p1.x() - p2.x();
    double y1 = p1.y() - p2.y();
    double x3 = p3.x() - p2.x();
    double y3 = p3.y() - p2.y();
    double dot = x1 * x3 + y1 * y3;
    double det = x1 * y3 - y1 * x3;
    double result = atan2(det, dot);
    return ((result < 0) ? (result * 180 / 3.141592) + 360 : (result * 180 / 3.141592));
}


void DCEL::displayDCEL(int  val)
{
    if (val & 0x01) print_vertices();
    if (val & 0x02) print_edges();
    if (val & 0x04) print_ordered_edges();
    if (val & 0x08) print_faces();
    if (val & 0x10) print_adj();

    if  (val & 0x80)
    {
        double threshold_area = 5.5;
        print_faces_with_area_lessthan_threshhold(threshold_area);
    }
    if (val & 0x100)
    {
        int index = 6;
        print_neighbouring_faces(edges[index]);
    }
}

VertexPtr DCEL::validAdjacent(const VertexPtr & vert)
{
    if (vert->adjacent_vertices.size() < 2)
    {
        VertexPtr dvp;
        return dvp;
    }

    int good = 0;
    VertexPtr valid_v;
    for (const auto & v : std::as_const(vert->adjacent_vertices))
    {
        if (!valid_v)
            valid_v = v.lock();    // first good one
        good++;
    }

    if (good > 1)
    {
        return valid_v;
    }
    else
    {
        VertexPtr dvp;
        return dvp;
    }
}

EdgePtr DCEL::findEdge(const VertexPtr &start , const VertexPtr&  end, bool expected)
{
    NeighboursPtr np = getNeighbours(start);

    for (WeakEdgePtr & wep : *np)
    {
        EdgePtr edge = wep.lock();
        if (edge->v1 == start && edge->v2 == end)
        {
            return edge;
        }
        edge = edge->twin.lock();
        if (edge->v1 == start && edge->v2 == end)
        {
            return edge;
        }
    }

    if (expected)
        qWarning() << "Error in serching edges";
    EdgePtr dep;
    return dep;
}

void DCEL::print_vertices()
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl << "********** VertexPtrTable ***********" << endl;
    deb << "Vertex" << " Coordinates " << "Incident Edge " << endl;
    for (auto & v : std::as_const(vertices))
    {
        deb << vertexIndex(v) << "\t(" << v->pt.x() << " , " << v->pt.y() << ")" << endl; ;
    }
    qDebug().noquote() << astring;
}

void DCEL::print_edges()
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl << "********** Edges  **********" << endl;
    for (auto & edge : std::as_const(edges))
    {
        print_edge_detail(edge, "E ", deb);
    }
    qDebug().noquote() << astring;
}

void DCEL::print_ordered_edges()
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl << "********** Ordered Edges  **********" << endl;
    deb << "Half-edge\tTwin\t\tIncident_Face Next\tPrevious" << endl;
    for (const auto & edge : std::as_const(edges))
    {
        print_edge(edge,deb);
        print_edge(edge->twin.lock(),deb);
        if (edge->incident_face.lock() != nullptr)
            deb << "   F" << faceIndex(edge->incident_face.lock()) << "\t    ";
        else
            deb << "   NULL" << "     ";
        EdgePtr next = edge->next.lock();
        if (next)
            print_edge(next,deb);
        else
            deb << "NONE  ";
        EdgePtr prev = edge->prev();
        if (prev)
            print_edge(prev,deb);
        else
            deb << "NONE";
        deb << endl;
    }
    qDebug().noquote() << astring;
}

void DCEL::print_faces()
{
    QString astring;
    QDebug  deb(&astring);

    deb <<endl << "*************** Face_Table **************" << endl;
    deb << "Face\t Size\t\tEdges" << endl;
    for (auto & aface : std::as_const(faces))
    {
        deb << "F" << faceIndex(aface);
        if (aface->outer)
            deb << " OUTER\t";
        else
            deb << "      \t";

        deb << " size = " << aface->area << "\t";

        EdgePtr head = aface->incident_edge.lock();
        if (head == nullptr)
        {
            deb << "NULL edge";
        }
        else
        {
            EdgePtr e = head;
            do
            {
                print_edge(e,deb);
                e = e->next.lock();
            } while (e != head);
        }
        deb << endl;
    }
    qDebug().noquote() << astring;
}

void DCEL::print_adj()
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl << "************ Adjacent Vertices *****" << endl;
    for (auto & v : std::as_const(vertices))
    {
        deb << "vertex:" << vertexIndex(v) << "count:" << v->adjacent_vertices.size() << " : ";
        if (v->adjacent_vertices.size())
        {
            for (auto & v2 : std::as_const(v->adjacent_vertices))
            {
                deb << vertexIndex(v2.lock()) << " ";
            }
        }
        else
        {
            deb << "NO adjacent vertices";
        }
        deb << endl;
    }
    qDebug().noquote() << astring;
}

void DCEL::print_faces_with_area_lessthan_threshhold(double threshhold_area)
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl << "************ Faces smaller than" << threshhold_area << " *****" << endl;
    for (auto & face : std::as_const(faces))
    {
        double area = face->area;
        if (area < threshhold_area && area > 0)
        {
            deb << "F" << faceIndex(face) << ", less than threshold." <<  endl;
        }
    }
    qDebug().noquote() << astring;
}

void DCEL::print_neighbouring_faces(const EdgePtr &edge)
{
    // cycle through the edges of the face, see which face is the twin is incident to
    // this also demonstrates the insanity of flipping between the two different edge structures

    QString astring;
    QDebug  deb(&astring);

    deb << endl << "*******Neighbouring faces to edge " << edgeIndex(edge) << " [" << vertexIndex(edge->v1)
        << "-" << vertexIndex(edge->v2) << "]" << endl;

    vector<FacePtr> neighbours;

    EdgePtr head = edge;
    EdgePtr twin = edge->twin.lock();

    FacePtr f = twin->incident_face.lock();
    if (f)
    {
        neighbours.push_back(f);
    }

    auto nedge = edge->next.lock();
    while (nedge != head)
    {
        twin = edge->twin.lock();
        f = twin->incident_face.lock();
        if (f && std::find(neighbours.begin(), neighbours.end(), f) == neighbours.end())
        {
            neighbours.push_back(f);
        }
        nedge = nedge->next.lock();
    }

    for (auto & f : std::as_const(neighbours))
    {
        deb << "F" << faceIndex(f);
        if (f->outer)
            deb << "   OUTER";
        deb << endl;
    }
    qDebug().noquote() << astring;

}

void DCEL::print_edge_detail(const EdgePtr &e, QString && name, QDebug & deb)
{
    deb  << name       << edgeIndex(e)
        << " origin " << vertexIndex(e->v1)
        << " end "    << vertexIndex(e->v2)
        << " next "   << (e->next.lock() ? edgeIndex(e->next.lock()) : -1);

    if (e->twin.lock() && e->next.lock())
    {
        deb << " prev "   << (e->prev() ? edgeIndex(e->prev()) : -1)
            << " twin "   << (e->twin.lock() ? edgeIndex(e->twin.lock()) : -1)
            << " face "   << (e->incident_face.lock() ? faceIndex(e->incident_face.lock()) : -1);
    }

    deb << endl;
}

void DCEL::print_edge(const EdgePtr & edge, QDebug & deb)
{
    if (edge)
        deb << edgeIndex(edge) << " [" << vertexIndex(edge->v1) << "-" << vertexIndex(edge->v2) <<"] \t";
    else
        deb << " NULL\t";
}

QString DCEL::info() const
{
    return QString("vertices=%1 edges=%2 faces=%3").arg(vertices.size()).arg(edges.size()).arg(faces.size());
}


