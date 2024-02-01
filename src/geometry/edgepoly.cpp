#include <QTransform>
#include <QPainter>
#include "geometry/arcdata.h"
#include "geometry/edgepoly.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/geo.h"
#include "misc/geo_graphics.h"

using std::make_shared;

EdgePoly::EdgePoly()
{
}

EdgePoly::EdgePoly(QPolygonF & poly)
{
    init(poly);
}

EdgePoly::EdgePoly(QRectF & rect)
{
    QPolygonF apoly(rect);
    init(apoly);
}

EdgePoly::EdgePoly(const Circle & circle)
{
    set(circle);
}

EdgePoly::EdgePoly(PolyPtr pp)
{
    QPolygonF & p = *pp.get();
    init(p);
}

void EdgePoly::set(QPolygonF & poly)
{
    init(poly);
}

EdgePoly::EdgePoly(const QVector<EdgePtr> & qvep)
{
    for (const auto & edge : std::as_const(qvep))
    {
        push_back(edge);
    }
}

void EdgePoly::set(QRectF & rect)
{
    QPolygonF poly(rect);
    init(poly);
}

void EdgePoly::set(const Circle &c)
{
    QPointF centre = c.centre;
    qreal   radius = c.radius;

    auto v1 = make_shared<Vertex>(centre + QPointF(-radius,0));
    auto v2 = make_shared<Vertex>(centre + QPointF(0,radius));
    auto v3 = make_shared<Vertex>(centre + QPointF(radius,0));
    auto v4 = make_shared<Vertex>(centre + QPointF(0,-radius));

    auto e1 = make_shared<Edge>(v1,v2,centre,true,false);
    auto e2 = make_shared<Edge>(v2,v3,centre,true,false);
    auto e3 = make_shared<Edge>(v3,v4,centre,true,false);
    auto e4 = make_shared<Edge>(v4,v1,centre,true,false);

    push_back(e1);
    push_back(e2);
    push_back(e3);
    push_back(e4);
}

void EdgePoly::init(QPolygonF & poly)
{
    clear();

    auto size = poly.size();
    if (size && poly.isClosed())
        size--;

    VertexPtr v  = make_shared<Vertex>(poly[0]);
    VertexPtr v1 = v;
    for (int i=1; i < size; i++)
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
    for (const auto & edge : std::as_const(*this))
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
        if  (edge->getType() == EDGETYPE_CURVE || edge->getType() == EDGETYPE_CHORD)
        {
            edge2->setArcCenter(edge->getArcCenter(),edge->isConvex(),(edge->getType()==EDGETYPE_CHORD));
        }
        epoly.push_back(edge2);
    }
    return epoly;
}

void EdgePoly::rotate(qreal angle)
{
    QPointF center = calcCenter();
    QTransform t;
    t.translate(center.x(), center.y());
    t.rotate(angle);
    t.translate(-center.x(), -center.y());
    mapD(t);
}

void EdgePoly::scale(qreal delta)
{
    QPointF center = calcCenter();
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
    for (const auto & edge : std::as_const(*this))
    {
        QPointF pt;

        VertexPtr v = edge->v1;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->pt = T.map(pt);
            mapped.push_back(v);
        }

        v = edge->v2;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->pt = T.map(pt);
            mapped.push_back(v);
        }

        if (edge->getType() == EDGETYPE_CURVE || edge->getType() == EDGETYPE_CHORD)
        {
            QPointF p3 = edge->getArcCenter();
            edge->setArcCenter(T.map(p3),edge->isConvex(),(edge->getType()==EDGETYPE_CHORD));
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
        if (e->getType() == EDGETYPE_CURVE || e->getType() == EDGETYPE_CHORD)
        {
            QPointF p3 = e->getArcCenter();
            ne->setArcCenter(T.map(p3),e->isConvex(),(e->getType()==EDGETYPE_CHORD));
        }
        ep.push_back(ne);
        v1 = v2;
    }

    EdgePtr e = edges.last();
    EdgePtr ne = make_shared<Edge>(v1,first);
    ne->setSwapState(e->getSwapState());
    if (e->getType() == EDGETYPE_CURVE || e->getType() == EDGETYPE_CHORD)
    {
        QPointF p3 = e->getArcCenter();
        ne->setArcCenter(T.map(p3),e->isConvex(),(e->getType()==EDGETYPE_CHORD));
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
    const QPolygonF poly = getPoints();
    return Geo::isClockwise(poly);
}

bool EdgePoly::isClockwiseK()
{
    QPolygonF poly = getPoints();
    return Geo::isClockwiseKaplan(poly);
}

bool EdgePoly::isCorrect()
{
    return (first()->v1 == last()->v2);
}

bool EdgePoly::isValid(bool rigorous)
{
    if (size() == 0)
    {
        return false;
    }

    if (!isCorrect())
    {
        return false;
    }

    QVector<VertexPtr> v1s;
    QVector<VertexPtr> v2s;

    VertexPtr v0 = last()->v2;
    for (auto & edge : std::as_const(*this))
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
    QPolygonF poly = getPoints();
    Q_ASSERT(!poly.isClosed());
    poly << poly[0];
    return poly;
}

QPolygonF EdgePoly::getPoints() const
{
    QPolygonF poly;
    for (const auto & edge : std::as_const(*this))
    {
        QPointF pt = edge->v1->pt;
        poly << pt;
    }
    return poly;
}

QPolygonF EdgePoly::getMids() const
{
    QPolygonF  mids;    // mid-points
    for (const auto & edge : std::as_const(*this))
    {
        mids <<  edge->getMidPoint();
    }
    return mids;
}

QRectF EdgePoly::getRect() const
{
    QRectF rect;
    QPolygonF poly = getPoly();
    if (poly.size() >= 3)
    {
        rect = QRectF(poly[0],poly[2]);
    }
    return rect;
}

QLineF EdgePoly::getEdge(int edge)
{
    QVector<EdgePtr> & edges = *this;
    EdgePtr e = edges[edge % edges.size()];
    return e->getLine();
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

void EdgePoly::paint(QPainter * painter, QTransform T, bool annotate)
{
    painter->save();
    QFont font = painter->font();
    font.setPixelSize(14);
    painter->setFont(font);
    int edgenum = 0;

    for (const auto & edge : std::as_const(*this))
    {
        QPointF p1 = T.map(edge->v1->pt);
        QPointF p2 = T.map(edge->v2->pt);

        if (edge->getType() == EDGETYPE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF arcCenter = T.map(edge->getArcCenter());
            ArcData ad(p1,p2,arcCenter,edge->isConvex());
            painter->drawArc(ad.rect, qRound(ad.start * 16.0),qRound(ad.span * 16.0));
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            QPointF arcCenter = T.map(edge->getArcCenter());
            ArcData ad(p1,p2,arcCenter,edge->isConvex());
            painter->drawChord(ad.rect, qRound(ad.start * 16.0),qRound(ad.span * 16.0));
        }

        if (annotate)
        {
            QPointF mid = (p1+p2)/2.0;
            painter->drawText(QPointF(mid.x()+13,mid.y()+13),QString::number(edgenum));
            edgenum++;
        }
    }
    painter->restore();
}

void EdgePoly::draw(GeoGraphics * gg, QPen & pen)
{
    for (const auto & edge : std::as_const(*this))
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            gg->drawArc(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),pen);
        }
        else if (edge->getType() == EDGETYPE_CHORD)
        {
            gg->drawChord(edge->v1->pt,edge->v2->pt,edge->getArcCenter(),edge->isConvex(),pen);
        }
        gg->drawCircle(edge->v1->pt,5,QPen(Qt::red),QBrush(Qt::red));
    }
}

void EdgePoly::drawPts(GeoGraphics * gg, QPen &pen)
{
    for (const auto & edge : std::as_const(*this))
    {
        gg->drawCircle(edge->v1->pt,3,pen, QBrush(pen.color()));
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
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr edge = *it;
        EdgePtr next;
        if ((it + 1) != end())
        {
            next = *(it+1);
        }
        else
        {
            next = first();
        }
        VertexPtr v = next->v1;
        edge->setV2(v);
    }
}

QVector<VertexPtr> EdgePoly::getVertices()
{
    QVector<VertexPtr> vec;
    for (const auto & edge : std::as_const(*this))
    {
        vec.push_back(edge->v1);
    }
    return vec;
}

QVector<QLineF> EdgePoly::getLines()
{
    QVector<QLineF> vec;
    for (const auto & edge : std::as_const(*this))
    {
        vec.push_back(QLineF(edge->v1->pt,edge->v2->pt));
    }
    return vec;
}

int EdgePoly::numSwapped()
{
    int num = 0;
    for (const auto & edge : std::as_const(*this))
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
    for (const auto & edge : *this)
    {
        qDebug().noquote() << edge->dump();
    }
}

QPointF EdgePoly::calcCenter()
{
    QPointF accum;
    for (const auto & edge : std::as_const(*this))
    {
        accum += edge->v1->pt;
    }
    QPointF cent = accum / static_cast<qreal>(size());
    return cent;
}

QPointF EdgePoly::calcIrregularCenter()
{
    double area = 0.0;
    double x0   = 0.0; // Current vertex X
    double y0   = 0.0; // Current vertex Y
    double x1   = 0.0; // Next vertex X
    double y1   = 0.0; // Next vertex Y
    double a    = 0.0;  // Partial signed area
    double sumX = 0.0;
    double sumY = 0.0;

    // For all vertices in a loop
    const auto & vertices = getVertices();
    Q_ASSERT(vertices.first()->pt != vertices.last()->pt);

    VertexPtr prev = vertices.last();
    for (const VertexPtr &  next : std::as_const(vertices))
    {
        x0 = prev->pt.x();
        y0 = prev->pt.y();
        x1 = next->pt.x();
        y1 = next->pt.y();
        a  = x0*y1 - x1*y0;
        area += a;
        sumX += ((x0 + x1)*a);
        sumY += ((y0 + y1)*a);
        prev = next;
    }

    area *= 0.5;

    QPointF centroid(sumX,sumY);
    centroid /= (6.0 * area);

    return centroid;
}
