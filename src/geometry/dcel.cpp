#include <deque>
#include <QDebug>
#include <QElapsedTimer>

#include "geometry/dcel.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/neighbours.h"
#include "misc/tpm_io.h"

using namespace std;

#define INF 10000

int DCEL::refs  = 0;

DCEL::DCEL(MapPtr map)
{
    qDebug() << "Constructing DCEL";

    vertices = map->vertices;
    edges    = map->edges;
    qDebug().noquote() << "DECL imported" << summary();

    nMap = map->getNeighbourMap();   // do this first before half-edges are added

    buildDCEL();

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

void DCEL::buildDCEL()
{
    QElapsedTimer timer;
    timer.start();

    // fill half edges
    QVector<EdgePtr> additions(edges.size());
    int i=0;
    for (auto e : edges)
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
    qDebug().noquote() << "DCEL loaded" << summary();

    fill_half_edge_table();
    qDebug() << "DCEL: half edge table filled";

    fill_half_edge_faces();
    qDebug() << "DCEL: face table filled";

    fill_face_table_inner_components();

    qreal time    = timer.elapsed();
    double qdelta = time /1000.0;
    QString sDelta = QString("%1").arg(qdelta, 8, 'f', 3);

    qDebug().noquote() << "DCEL complete time =" << sDelta;;
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
    for (const auto & v : qAsConst(vert->adjacent_vertices))
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

void DCEL::fill_half_edge_table()
{
    qDebug().noquote() << "start fill_half_edge_table() edges =" << edges.size();

    //int index = 0;
    QVector<EdgePtr> deletions;
    for (const auto & edge : qAsConst(edges))
    {
        //qDebug() << "dcel index" << index++;
        if (!edge->dvisited)
        {
            edge->dvisited = true;

            EdgePtr current = edge;
            EdgePtr next;
            while ((next = next_half_edge(current)) != edge)
            {
                if (!next)
                {
                    deletions.push_back(current);
                    break;
                }
                next->dvisited = true;
                current->next = next;
                current       = next;
            }
            current->next = edge;   // same as next
        }
    }

    for (auto&  edge2 : qAsConst(deletions))
    {
        EdgePtr edget = edge2->twin.lock();
        edges.removeAll(edge2);
        edges.removeAll(edget);
    }

    qDebug().noquote() << "end fill_half_edge_table() edges =" << edges.size();
}

EdgePtr DCEL::next_half_edge(const EdgePtr &current)
{
    double max_angle = 0;

    VertexPtr start = current->v1;
    VertexPtr end   = current->v2;
    QPointF p1      = start->pt;
    QPointF p2      = end->pt;

    VertexPtr next_vertex;
    for (const auto & wv : qAsConst(end->adjacent_vertices))
    {
        VertexPtr v = wv.lock();
        if (v == end)
        {
            continue;
        }
        else
        {
            double temp_angle = angle(p1, p2, v->pt);
            if(max_angle < temp_angle)
            {
                max_angle = temp_angle;
                next_vertex = v;
            }
        }
    }
    if (next_vertex)
    {
        return findEdge(end, next_vertex,true);
    }
    else
    {
        EdgePtr dp;
        return dp;     // edge is not connected
    }
}

void DCEL::fill_half_edge_faces()
{
    for (const auto & e :qAsConst(edges))
    {
        e->dvisited = false;
    }

    for (const auto & edge : qAsConst(edges))
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
    for (const auto & hedge : qAsConst(edges))
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

int DCEL::vertexIndex(const VertexPtr & v)
{
    for (int i = 0; i < vertices.size(); i++)
    {
        if (vertices[i] == v)
        {
            return i;
		}
    }
    qWarning() << "Error in vertexIndex";
    return -1;
}

EdgePtr DCEL::findEdge(const VertexPtr &start , const VertexPtr&  end, bool expected)
{
    NeighboursPtr np = nMap->getNeighbours(start);
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

int DCEL::edgeIndex(const EdgePtr &edge)
{
    for (int i = 0; i < edges.size(); i++)
    {
        if (edges[i] == edge)
        {
            return i;
        }
    }
    qWarning() << "Error in edgeIndex";
    return -1;
}

int DCEL::faceIndex(const FacePtr &aface)
{
    for (int i = 0; i < faces.size(); i++)
    {
        if (faces[i] == aface)
        {
            return i;
        }
    }
    qWarning() << "Error in search_faces";
    return -1;
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

    for (auto & aface : qAsConst(faces))
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
    for (auto face : qAsConst(faces))
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

void DCEL::print_vertices()
{
    QString astring;
    QDebug  deb(&astring);

    deb << endl << "********** VertexPtrTable ***********" << endl;
    deb << "Vertex" << " Coordinates " << "Incident Edge " << endl;
    for (auto & v : qAsConst(vertices))
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
    for (auto & edge : qAsConst(edges))
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
    for (auto edge : qAsConst(edges))
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
    for (auto & aface : qAsConst(faces))
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
    for (auto & v : qAsConst(vertices))
    {
        deb << "vertex:" << vertexIndex(v) << "count:" << v->adjacent_vertices.size() << " : ";
        if (v->adjacent_vertices.size())
        {
            for (auto & v2 : qAsConst(v->adjacent_vertices))
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
    for (auto & face : qAsConst(faces))
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

    for (auto & f : neighbours)
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

QString DCEL::summary() const
{
    return QString("vertices=%1 edges=%2").arg(vertices.size()).arg(edges.size());
}
