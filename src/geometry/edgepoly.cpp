#include <QTransform>
#include <QPainter>
#include "geometry/edgepoly.h"
#include "geometry/Transform.h"
#include "base/utilities.h"
#include "viewers/GeoGraphics.h"

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
        push_back(make_shared<Edge>(v1,v2));
        v1 = v2;
    }
    push_back(make_shared<Edge>(v1,v));
}

void EdgePoly::rotate(qreal angle)
{
    QTransform T = QTransform().rotate(angle);
    mapD(T);
}

void EdgePoly::mapD(QTransform T)
{
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr e = *it;
        VertexPtr v1 = e->getV1();
        v1->setPosition(T.map(v1->getPosition()));
    }

    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr e = *it;
        if (e->getType() == EDGE_CURVE)
        {
            QPointF p3 = e->getArcCenter();
            e->setArcCenter(T.map(p3),e->isConvex());
        }
    }
}

EdgePoly EdgePoly::map(QTransform T)
{
    EdgePoly newp;
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr e = *it;
        EdgePtr e2;
        VertexPtr v1 = make_shared<Vertex>(T.map(e->getV1()->getPosition()));
        VertexPtr v2 = make_shared<Vertex>(T.map(e->getV2()->getPosition()));
        if (e->getType() == EDGE_CURVE)
        {
            QPointF p3 = T.map(e->getArcCenter());
            e2 = make_shared<Edge>(v1,v2,p3, e->isConvex());
        }
        else
        {
            e2 = make_shared<Edge>(v1,v2);
        }
        newp.push_back(e2);
    }

    return newp;
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

bool EdgePoly::isClockwise()
{
    QPolygonF poly = getPoly();
    return Utils::isClockwise(poly);
}

QPolygonF EdgePoly::getPoly()
{
    QPolygonF poly;
    for (auto it = begin(); it != end(); it++)
    {
        EdgePtr e = *it;
        QPointF pt = e->getV1()->getPosition();
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
    QPointF p1 = e->getV1()->getPosition();
    QPointF p2 = e->getV2()->getPosition();
    e = edges[++edge % edges.size()];
    QPointF p3 = e->getV2()->getPosition();

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

        QPointF p1 = T.map(edge->getV1()->getPosition());
        QPointF p2 = T.map(edge->getV2()->getPosition());

        if (edge->getType() == EDGE_LINE)
        {
            painter->drawLine(p1,p2);
        }
        else if (edge->getType() == EDGE_CURVE)
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
        if (edge->getType() == EDGE_LINE)
        {
            gg->drawLine(edge->getLine(),pen);
        }
        else if (edge->getType() == EDGE_CURVE)
        {
            gg->drawChord(edge->getV1()->getPosition(),edge->getV2()->getPosition(),edge->getArcCenter(),pen,QBrush(),edge->isConvex());
        }
    }
}
