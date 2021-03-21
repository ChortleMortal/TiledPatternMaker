#include <QTransform>
#include <QPainter>
#include "geometry/edgepoly.h"
#include "geometry/transform.h"
#include "base/utilities.h"
#include "viewers/geo_graphics.h"

EdgePoly::EdgePoly()
{

}

EdgePoly::EdgePoly(QPolygonF & poly)
{
    init(poly);
}

EdgePoly::EdgePoly(PolyPtr pp)
{
    QPolygonF & p = *pp.get();
    init(p);
}

void EdgePoly::init(QPolygonF & poly)
{
    Q_ASSERT(!poly.isClosed());

    VertexPtr v  = make_shared<Vertex>(poly[0]);
    VertexPtr v1 = v;
    for (int i=1; i < poly.size(); i++)
    {
        VertexPtr v2 = make_shared<Vertex>(poly[i]);
        EdgePtr e = make_shared<Edge>(v1,v2);
        push_back(e);
        v1 = v2;
    }
    EdgePtr e = make_shared<Edge>(v1,v);
    push_back(e);
}

// makes a new edge poly
EdgePoly EdgePoly::recreate() const
{
    EdgePoly epoly;
    QMap<VertexPtr,VertexPtr> vmap;
    for (auto edge : *this)
    {
        VertexPtr oldv1 = edge->v1;
        VertexPtr newv1;
        VertexPtr oldv2 = edge->v2;
        VertexPtr newv2;
        if (vmap.contains(oldv1))
        {
            newv1 = vmap.value(oldv1);
        }
        else
        {
            newv1 = make_shared<Vertex>(oldv1->pt);
            vmap[oldv1] = newv1;
        }
        if (vmap.contains(oldv2))
        {
            newv2 = vmap.value(oldv2);
        }
        else
        {
            newv2 = make_shared<Vertex>(oldv2->pt);
            vmap[oldv2] = newv2;
        }
        EdgePtr edge2 = make_shared<Edge>(newv1,newv2);
        if  (edge->getType() == EDGETYPE_CURVE)
        {
            edge2->setArcCenter(edge->getArcCenter(),edge->isConvex());
        }
        epoly.push_back(edge2);
    }
    return epoly;
}

void EdgePoly::rotate(qreal angle)
{
    QPointF center = Point::center(*this);
    QTransform t;
    t.translate(center.x(), center.y());
    t.rotate(angle);
    t.translate(-center.x(), -center.y());
    mapD(t);
}

void EdgePoly::scale(qreal delta)
{
    QPointF center = Point::center(*this);
    QTransform t;
    t.translate(center.x(), center.y());
    t.scale(delta,delta);
    t.translate(-center.x(), -center.y());
    mapD(t);
}

void EdgePoly::mapD(QTransform T)
{
    // Can't assume is regular poly
    QVector<VertexPtr> mapped;
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr e = *it;
        QPointF pt;

        VertexPtr v = e->v1;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->setPosition(T.map(pt));
            mapped.push_back(v);
        }

        v = e->v2;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->setPosition(T.map(pt));
            mapped.push_back(v);
        }

        if (e->getType() == EDGETYPE_CURVE)
        {
            QPointF p3 = e->getArcCenter();
            e->setArcCenter(T.map(p3),e->isConvex());
        }
    }
}

EdgePoly EdgePoly::map(QTransform T) const
{
    // makes a new EdgePoly
    VertexPtr first;
    VertexPtr v1;
    VertexPtr v2;

    EdgePoly ep;
    QPointF pt;
    const QVector<EdgePtr> & edges = *this;

    EdgePtr efirst = edges[0];
    pt = efirst->v1->pt;
    first = make_shared<Vertex>(T.map(pt));
    v1 = first;

    for (auto i = 0; i < (edges.size()-1); i++)
    {
        EdgePtr e = edges[i];
        pt = e->v2->pt;
        VertexPtr v2 = make_shared<Vertex>(T.map(pt));

        EdgePtr ne = make_shared<Edge>(v1,v2);
        ne->setSwapState(e->getSwapState());
        if (e->getType() == EDGETYPE_CURVE)
        {
            QPointF p3 = e->getArcCenter();
            ne->setArcCenter(T.map(p3),e->isConvex());
        }
        ep.push_back(ne);
        v1 = v2;
    }

    EdgePtr e = edges.last();
    EdgePtr ne = make_shared<Edge>(v1,first);
    ne->setSwapState(e->getSwapState());
    if (e->getType() == EDGETYPE_CURVE)
    {
        QPointF p3 = e->getArcCenter();
        ne->setArcCenter(T.map(p3),e->isConvex());
    }
    ep.push_back(ne);

    return ep;
}

bool EdgePoly::equals(const EdgePoly & other)
{
    if (size() != other.size())
        return false;

    for (int i=0; i < size(); i++)
    {
        EdgePtr edge  = at(i);
        EdgePtr oedge = other.at(i);
        if (!edge->equals(oedge))
            return false;
    }
    return true;
}

bool EdgePoly::isClockwise() const
{
    const QPolygonF poly = getPoly();
    return Utils::isClockwise(poly);
}

bool EdgePoly::isValid(bool rigorous)
{
    if (size() == 0)
    {
        return false;
    }

    QVector<VertexPtr> v1s;
    QVector<VertexPtr> v2s;

    VertexPtr v0 = last()->v2;
    for (auto edge : qAsConst(*this))
    {
        VertexPtr v1 = edge->v1;
        VertexPtr v2 = edge->v2;

        if (rigorous)
        {
            if (v1s.contains(v1))
            {
                qWarning() << "EdgePoly v1 error";
                return false;
            }
            if (v2s.contains(v2))
            {
                qWarning() << "Edgepoly v2 error";
                return false;
            }

            v1s.push_back(v1);
            v2s.push_back(v2);
        }

        if (v1 != v0)
        {
            qWarning() << "EdgePoly sequence error";
            return false;
        }
        v0 = v2;
    }
    return true;
}

QPolygonF EdgePoly::getPoly() const
{
    QPolygonF poly;
    for (auto edge : *this)
    {
        QPointF pt = edge->v1->pt;
        poly << pt;
    }
    return poly;
}

qreal EdgePoly::getAngle(int edge)
{
    // calculate the inner product
    QVector<EdgePtr> & edges = *this;
    if (edge >= edges.size())
        return 0.0;

    EdgePtr e = edges[edge];
    QPointF p1 = e->v1->pt;
    QPointF p2 = e->v2->pt;
    e = edges[++edge % edges.size()];
    QPointF p3 = e->v2->pt;

    qreal dx21 = p2.x()-p1.x();
    qreal dx31 = p3.x()-p1.x();
    qreal dy21 = p2.y()-p1.y();
    qreal dy31 = p3.y()-p1.y();
    qreal m12 = sqrt( dx21*dx21 + dy21*dy21 );
    qreal m13 = sqrt( dx31*dx31 + dy31*dy31 );
    qreal theta = acos( (dx21*dx31 + dy21*dy31) / (m12 * m13) );
    return qRadiansToDegrees(theta);
}

void EdgePoly::paint(QPainter * painter, QTransform T)
{
    for(auto e = begin(); e != end(); e++)
    {
        EdgePtr edge = *e;

        QPointF p1 = T.map(edge->v1->pt);
        QPointF p2 = T.map(edge->v2->pt);

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF ArcCenter = T.map(edge->getArcCenter());
            arcData ad = edge->calcArcData(p1,p2,ArcCenter,edge->isConvex());
            painter->drawArc(ad.rect, qRound(ad.start * 16.0),qRound(ad.span * 16.0));
        }
    }
}

void EdgePoly::draw(GeoGraphics * gg, QPen pen)
{
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr edge = *it;
        if (edge->getType() == EDGETYPE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            gg->drawChord(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),pen,QBrush(),edge->isConvex());
        }
    }
}

void EdgePoly::reverseWindingOrder()
{
    QVector<EdgePtr> orig = *this;
    clear();
    for (auto it = orig.rbegin(); it != orig.rend(); it++)
    {
        auto edge = *it;
        push_back(edge);
    }
    relink();
}

void EdgePoly::relink()
{
    QVector<EdgePtr> & epoly = *this;
    for (auto it = epoly.begin(); it != epoly.end(); it++)
    {
        EdgePtr edge = *it;
        EdgePtr next;
        if ((it + 1) != epoly.end())
        {
            next = *(it+1);
        }
        else
        {
            next = epoly.first();
        }
        VertexPtr v = next->v1;
        edge->setV2(v);
    }
}

QVector<VertexPtr> EdgePoly::getVertices()
{
    QVector<VertexPtr> vec;
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr edge = *it;
        vec.push_back(edge->v1);
    }
    return vec;
}

int EdgePoly::numSwapped()
{
    int num = 0;
    for (auto edge : qAsConst(*this))
    {
        if (edge->getSwapState())
        {
            num++;
        }
    }
    return num;
}

void EdgePoly::dump() const
{
    qDebug() << "EdgePoly::dump()" << ((isClockwise()) ? "Clockwise" : "Anticlockwise");
    for (auto edge : *this)
    {
        edge->dump();
    }
}
