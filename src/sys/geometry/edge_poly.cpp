#include <QTransform>
#include <QPainter>
#include <QVector>

#include "gui/viewers/geo_graphics.h"
#include "sys/geometry/arcdata.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/vertex.h"

using std::make_shared;

/*
 *  Why does this class even exist? What is it for? What does it do?
 *  Well, it is a vector of shared edge pointers.  So whey not a vector of QLineFs or a QPolygonF?
 *  Well, the edges contain shared vertex pointers,
 *        and the vertex at the end of one line is the same as the vertex at the start of the next line
 *        and the first vertex and the last vertex are the same - the edge poly is alwys 'closed' hence isCorrect()
 *  Isn't this reminiscent of maps which have vertices and edges?
 *  No, maps are quite different and EdgeePolys are not used in maps.  It is temping to put a getMap() function into
 *      EdgePoly, but I have resisted to make the difference clear.
 *  Unlike large maps where we want to minimise the number of shared vertices and edges, here we create anddestory them frequently
 *  Internally, there is a base edge set and the calculated epoly edge set. The epoly is calculated ( compose() )from the base plus scale & rotation.
 *  So we have
 *      Constructors    all call create
 *      Copy constuctor copies scale and rot, but recreates base and epoly, so retains nay maping of epoly for placement and/or drawoing
 *      = operator      same as copy consstructor
 *      create          initialises scale and rotate then calls set
 *      set             sets the base and composes
 *      compose         creates new edges and vertices from base+scale+rot
 *      decompsoe
 *      map             makes a copy and operates directly on copied epoly
 *      mapD            opertaes directly on epoly
 */

EdgePoly::EdgePoly()
{
    rotation = 0;
    scale    = 1.0;
}

EdgePoly::EdgePoly(const EdgePoly & ep)
{
    recreate(ep.base,base);
    recreate(ep.epoly,epoly);
    rotation = ep.rotation;
    scale = ep.scale;
    // does not compose
}

EdgePoly::EdgePoly(const EdgeSet & edgeSet)
{
    create(edgeSet);
}

EdgePoly::EdgePoly(const LineSet & lineSet, bool ordered)
{
    create(lineSet,ordered);
}

EdgePoly::EdgePoly(const QPolygonF &poly)
{
    create(poly);
}

EdgePoly::EdgePoly(const QRectF &rect)
{
    create(rect);
}

EdgePoly::EdgePoly(const Circle & circle)
{
    create(circle);
}

EdgePoly::EdgePoly(const PolyPtr pp)
{
    create(pp);
}

EdgePoly & EdgePoly::operator=(const EdgePoly & other)
{
    if (this != &other)
    {
        recreate(other.base,base);
        recreate(other.epoly,epoly);
        rotation = other.rotation;
        scale    = other.scale;
        // does not compose
    }
    return *this;
}

void EdgePoly::create(const EdgeSet & edgeSet)
{
    rotation = 0;
    scale    = 1.0;
    set(edgeSet);
}

void EdgePoly::create(const LineSet & lineSet, bool ordered)
{
    rotation = 0;
    scale    = 1.0;
    set(lineSet,ordered);
}

void EdgePoly::create(const QPolygonF & poly)
{
    rotation = 0;
    scale    = 1.0;
    set(poly);
}

void EdgePoly::create(QRectF & rect)
{
    rotation = 0;
    scale    = 1.0;
    set(rect);
}

void EdgePoly::create(const Circle & circle)
{
    rotation = 0;
    scale    = 1.0;
    set(circle);
}

void EdgePoly::create(PolyPtr pp)
{
    rotation = 0;
    scale    = 1.0;

    QPolygonF  poly = *pp.get();
    set(poly);
}

EdgePoly::~EdgePoly()
{
    base.clear();   // paranoia or needed?
    epoly.clear();
}

bool EdgePoly::operator == (const EdgePoly & other) const
{
    if (   getBasePoints() == other.getBasePoints()
        && Loose::equals(rotation,other.rotation)
        && Loose::equals(scale,other.scale))
        return true;
    else
        return false;
}

void EdgePoly::set(const EdgeSet & edgeSet)
{
    base     = edgeSet;     // this is a straight copy
    compose();
}

void EdgePoly::set(const LineSet & lineSet, bool ordered)
{
    // the goal here is to make a QPolygonF
    QPolygonF poly;

    LineSet lines = lineSet;    // now not const

    if (!ordered)
    {
        // create an ordered list first

        // Start with the first line
        QLineF current = lines.takeFirst();
        poly << current.p1();
        poly << current.p2();

        QPointF tail = current.p2();

        // Greedy chaining
        while (!lines.isEmpty())
        {
            bool extended = false;

            for (int i = 0; i < lines.size(); ++i)
            {
                const QLineF &ln = lines[i];

                if (ln.p1() == tail)
                {
                    poly << ln.p2();
                    tail = ln.p2();
                    lines.removeAt(i);
                    extended = true;
                    break;
                }
                else if (ln.p2() == tail)
                {
                    poly << ln.p1();
                    tail = ln.p1();
                    lines.removeAt(i);
                    extended = true;
                    break;
                }
            }

            if (!extended)
                break; // cannot chain further
        }
    }
    else
    {
        poly<< lines[0].p1();
        for (int i = 0; i < lines.size(); ++i)
        {
            poly << lines[i].p2();
        }
    }
    set(poly);
}

void EdgePoly::set(const QPolygonF & poly)
{
    createBase(poly);
    compose();
}

void EdgePoly::set(QRectF & rect)
{
    QPolygonF poly(rect);
    createBase(poly);
    compose();
}

void EdgePoly::set(const Circle &c)
{
    QPointF centre = c.centre;
    qreal   radius = c.radius;

    auto v1 = make_shared<Vertex>(centre + QPointF(-radius,0));
    auto v2 = make_shared<Vertex>(centre + QPointF(0,radius));
    auto v3 = make_shared<Vertex>(centre + QPointF(radius,0));
    auto v4 = make_shared<Vertex>(centre + QPointF(0,-radius));

    auto e1 = make_shared<Edge>(v1,v2,centre,CURVE_CONVEX);
    auto e2 = make_shared<Edge>(v2,v3,centre,CURVE_CONVEX);
    auto e3 = make_shared<Edge>(v3,v4,centre,CURVE_CONVEX);
    auto e4 = make_shared<Edge>(v4,v1,centre,CURVE_CONVEX);

    base.push_back(e1);
    base.push_back(e2);
    base.push_back(e3);
    base.push_back(e4);

    compose();
}

void EdgePoly::createBase(const QPolygonF &poly)
{
    base.clear();

    auto size = poly.size();
    if (size && poly.isClosed())
        size--;

    VertexPtr v  = make_shared<Vertex>(poly[0]);
    VertexPtr v1 = v;
    for (int i=1; i < size; i++)
    {
        VertexPtr v2 = make_shared<Vertex>(poly[i]);
        EdgePtr e = make_shared<Edge>(v1,v2);
        base.push_back(e);
        v1 = v2;
    }
    EdgePtr e = make_shared<Edge>(v1,v);
    base.push_back(e);
}

void EdgePoly::copyFromBase()
{
    recreate(base,epoly);
}

EdgePoly EdgePoly::recreate() const
{
    EdgePoly ep;
    ep.recreate(base,ep.base);
    ep.rotation = rotation;
    ep.scale    = scale;
    ep.compose();
    return ep;
}

void  EdgePoly::recreate(const EdgeSet & from, EdgeSet & to)
{
    to.clear();

    QMap<VertexPtr,VertexPtr> vmap;
    for (const auto & edge : from)
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
            edge2->chgangeToCurvedEdge(edge->getArcCenter(),edge->getCurveType());
        }
        to.push_back(edge2);
    }
}

void EdgePoly::clear()
{
    rotation = 0.0;
    scale    = 1.0;
    epoly.clear();
    base.clear();
}

void EdgePoly::compose()
{
    // creates the vector from the base+scale+rot
    QTransform t = getEpolyTransform();
    recreate(base,epoly);
    Q_ASSERT(isCorrect());
    if (!t.isIdentity())
    {
        mapD(t);
    }
    Q_ASSERT(isCorrect());
}

void EdgePoly::decompose()
{
    recreate(epoly,base);
    QTransform t = getEpolyTransform().inverted();   // pivot is unchanged by scale and rotation
    Q_ASSERT(isCorrect());
    if (!t.isIdentity())
    {
        mapDB(t);
    }
    Q_ASSERT(isCorrect());
}

void EdgePoly::setRotate(qreal angle)
{
    rotation = angle;
    compose();
}

void EdgePoly::setScale(qreal not_delta)
{
    scale = not_delta;
    compose();
}

void EdgePoly::mapD(QTransform T)
{
    // Can't assume is regular poly
    QVector<VertexPtr> mapped;
    for (auto & edge : epoly)
    {
        QPointF pt;

        VertexPtr v = edge->v1;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->setPt(T.map(pt));
            mapped.push_back(v);
        }

        v = edge->v2;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->setPt(T.map(pt));
            mapped.push_back(v);
        }

        if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF p3 = edge->getArcCenter();
            edge->chgangeToCurvedEdge(T.map(p3),edge->getCurveType());
        }
    }
}

void EdgePoly::mapDB(QTransform T)
{
    // Can't assume is regular poly
    QVector<VertexPtr> mapped;
    for (auto & edge : base)
    {
        QPointF pt;

        VertexPtr v = edge->v1;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->setPt(T.map(pt));
            mapped.push_back(v);
        }

        v = edge->v2;
        if (!mapped.contains(v))
        {
            pt = v->pt;
            v->setPt(T.map(pt));
            mapped.push_back(v);
        }

        if (edge->getType() == EDGETYPE_CURVE)
        {
            QPointF p3 = edge->getArcCenter();
            edge->chgangeToCurvedEdge(T.map(p3),edge->getCurveType());
        }
    }
}

EdgePoly EdgePoly::map(QTransform T) const
{
    // makes a new EdgePoly
    EdgePoly ep(*this);
    ep.mapD(T);
    return ep;
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

bool EdgePoly::baseContains(const EdgePtr ep)
{
    for (auto & edge : base)
    {
        if (edge == ep)
            return true;
    }
    return false;
}

bool EdgePoly::epolyContains(const EdgePtr ep)
{
    for (auto & edge : epoly)
    {
        if (edge == ep)
            return true;
    }
    return false;
}

bool EdgePoly::isCorrect()
{
    if (epoly.first()->v1 != epoly.last()->v2)
    {
        qWarning() << "first and last not the same";
        return false;
    }
    if (epoly.size() != base.size())
    {
        qWarning() << "base size" << base.size() << "epoly size" << epoly.size();
        return false;
    }
    return true;
}

bool EdgePoly::isValid(bool rigorous)
{
    if (epoly.size() == 0)
    {
        return false;
    }

    if (!isCorrect())
    {
        return false;
    }

    QVector<VertexPtr> v1s;
    QVector<VertexPtr> v2s;

    VertexPtr v0 = epoly.last()->v2;
    for (auto & edge : epoly)
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

QPolygonF EdgePoly::getPolygon() const
{
    QPolygonF poly = getPoints();
    Q_ASSERT(!poly.isClosed());
    poly << poly[0];
    return poly;
}

QPolygonF EdgePoly::getPoints() const
{
    QPolygonF poly;
    for (const auto & edge : epoly)
    {
        poly << edge->v1->pt;
    }
    return poly;
}

QPolygonF EdgePoly::getBasePoints() const
{
    QPolygonF poly;
    for (const auto & edge : base)
    {
        poly << edge->v1->pt;
    }
    return poly;
}

QPolygonF EdgePoly::getMids() const
{
    QPolygonF  mids;    // mid-points
    for (const auto & edge : epoly)
    {
        mids <<  edge->getMidPoint();
    }
    return mids;
}

QRectF EdgePoly::getRect() const
{
    QRectF rect;
    QPolygonF poly = getPolygon();
    if (poly.size() >= 3)
    {
        rect = QRectF(poly[0],poly[2]);
    }
    return rect;
}

QLineF EdgePoly::getEdge(int edge)
{
    EdgeSet & edges = epoly;
    EdgePtr e = edges[edge % edges.size()];
    return e->getLine();
}

// scale and rotation around the base centroid
QTransform EdgePoly::getEpolyTransform()
{
    QTransform t = QTransform::fromScale(scale,scale).rotate(rotation);
    if (t.isIdentity())
        return t;

    QPointF c = pivot(getBasePoints());
    return QTransform::fromTranslate(-c.x(),-c.y()) * t * QTransform::fromTranslate(c.x(),c.y());
}

// centroid, snapped to the origin when within tolerance (regular bases are centered there)
QPointF EdgePoly::pivot(const QPolygonF & pts)
{
    if (pts.isEmpty())
        return QPointF();

    QPointF c = centroid(pts);
    return Loose::zero(c) ? QPointF() : c;
}

qreal EdgePoly::getAngle(int edge)
{
    // calculate the inner product
    EdgeSet & edges = epoly;
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

QPainterPath EdgePoly::getPainterPath() const
{
    QPainterPath pp;

    auto e = epoly.first();
    pp.moveTo(e->v1->pt);

    for (auto & edge : epoly)
    {
        if (edge->getType() == EDGETYPE_LINE)
        {
            pp.lineTo(edge->v2->pt);
        }
        else if (edge->getType() == EDGETYPE_CURVE)
        {
            ArcData & ad = edge->getArcData();
            pp.arcTo(ad.rect(), ad.start(),ad.span());
        }
    }
    return pp;
}

void EdgePoly::paint(QPainter * painter, QTransform T, bool annotate) const
{
    painter->save();
    QFont font = painter->font();
    font.setPixelSize(14);
    painter->setFont(font);
    int edgenum = 0;

    for (auto & edge : epoly)
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
            ArcData ad(QLineF(p1,p2),arcCenter,edge->getCurveType());
            painter->drawArc(ad.rect(), qRound(ad.start() * 16.0),qRound(ad.span() * 16.0));
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

void EdgePoly::reverseWindingOrder()
{
    decompose();
    EdgeSet orig = base;    // makes a copy
    base.clear();
    for (auto it = orig.rbegin(); it != orig.rend(); it++)
    {
        auto edge = *it;
        base.push_back(edge);
    }
    relink();
    compose();
}

void EdgePoly::relink()
{
    for (auto it = base.begin(); it != base.end(); it++)
    {
        EdgePtr edge = *it;
        EdgePtr next;
        if ((it + 1) != base.end())
        {
            next = *(it+1);
        }
        else
        {
            next = base.first();
        }
        VertexPtr v = next->v1;
        edge->setV2(v);
    }
}

QVector<VertexPtr> EdgePoly::getVertices()
{
    QVector<VertexPtr> vec;
    for (auto & edge : epoly)
    {
        vec.push_back(edge->v1);
    }
    return vec;
}

QVector<QLineF> EdgePoly::getLines()
{
    QVector<QLineF> vec;
    for (auto & edge : epoly)
    {
        vec.push_back(edge->getLine());
    }
    return vec;
}

void EdgePoly::dumpPts() const
{
    qDebug() << "EdgePoly::dumpPts" << ((isClockwise()) ? "Clockwise" : "Anticlockwise") << "rotation" << rotation << "scale" << scale;
    qDebug() << "base " << getBasePoints();
    qDebug() << "epoly" << getPoints();
}

void EdgePoly::dumpBaseEdges() const
{
    qDebug() << "EdgePoly::dumpBaseEdges" << ((isClockwise()) ? "Clockwise" : "Anticlockwise") << "rotation" << rotation << "scale" << scale;
    for (auto & edge : base)
    {
        edge->dump();
    }
}

void EdgePoly::dumpEpolyEdges() const
{
    qDebug() << "EdgePoly::dumpEpolyEdges" << ((isClockwise()) ? "Clockwise" : "Anticlockwise") << "rotation" << rotation << "scale" << scale;
    for (auto & edge : epoly)
    {
        edge->dump();
    }
}

QPointF EdgePoly::calcCenter()
{
    QPointF accum;
    for (auto & edge : epoly)
    {
        accum += edge->v1->pt;
    }
    QPointF cent = accum / static_cast<qreal>(epoly.size());
    return cent;
}

QPointF EdgePoly::calcIrregularCenter()
{
    return centroid(getPoints());
}

// area centroid of a polygon, or the vertex average when it has no area
QPointF EdgePoly::centroid(const QPolygonF & pts)
{
    double area = 0.0;
    double sumX = 0.0;
    double sumY = 0.0;

    QPointF prev = pts.last();
    for (const QPointF & next : pts)
    {
        double a = prev.x()*next.y() - next.x()*prev.y();   // partial signed area
        area += a;
        sumX += (prev.x() + next.x()) * a;
        sumY += (prev.y() + next.y()) * a;
        prev = next;
    }

    if (Loose::zero(area))
    {
        QPointF accum;
        for (const QPointF & pt : pts)
            accum += pt;
        return accum / static_cast<qreal>(pts.size());
    }

    area *= 0.5;

    QPointF c(sumX,sumY);
    c /= (6.0 * area);
    return c;
}
