#ifdef __linux__
#include <cfloat>
#endif
#include <QPolygonF>
#include "geometry/belowandaboveedge.h"
#include "geometry/arcdata.h"
#include "geometry/geo.h"

bool BelowAndAbove::validate()
{
    if (!Geo::isValid(above) || !Geo::isValid(below))
   {
       return false;
   }
   return true;
}

QPolygonF BelowAndAboveEdge::getPoly() const
{
    QPolygonF p;
    p << v2.below << v2.v << v2.above << v1.below << v1.v << v1.above;
    if (!Geo::isClockwise(p))
        qWarning() << "Poly is CCW";
    return p;
}

QPainterPath BelowAndAboveEdge::getPainterPath()
{
    QPainterPath path;

    if (type == EDGETYPE_LINE)
    {
        path.moveTo(v2.below);
        path.lineTo(v2.v);
        path.lineTo(v2.above);
        path.lineTo(v1.below);
        path.lineTo(v1.v);
        path.lineTo(v1.above);
        path.lineTo(v2.below);
        return path;
    }
    else if (type == EDGETYPE_CURVE)
    {
        path.moveTo(v2.below);
        path.lineTo(v2.v);
        path.lineTo(v2.above);

        ArcData ad1(v2.above,v1.below,arcCenter,convex);
        path.arcTo(ad1.rect,ad1.start,-ad1.span);

        path.lineTo(v1.v);
        path.lineTo(v1.above);

        ArcData ad2(v1.above,v2.below,arcCenter,convex);
        path.arcTo(ad2.rect,ad2.start,ad2.span);
    }
    else if (type == EDGETYPE_CHORD)
    {
        qWarning("BelowAndAboveEdge - unexpected EDGETYPE_CHORD");
    }
    return path;
}

void BelowAndAboveEdge::dump(int idx)
{
    qDebug() << idx << "bae-v1" << v1.above << v1.v << v1.below;
    qDebug() << idx << "bae-v2" << v2.above << v2.v << v2.below;
}

void BelowAndAboveEdge::dumpV(int idx)
{
    qDebug() << idx << "v1.v=" << v1.v << "v2.v=" << v2.v;
}

bool BelowAndAboveEdge::validate(int idx)
{
    bool rv = true;

    if (!v1.validate())
       rv = false;
    if (!v2.validate())
        rv = false;
#if 0
    qreal a;
    qreal b;

    a = QLineF(v1.above,v1.v).length();
    b = QLineF(v1.v,v1.below).length();
    if (!Loose::equals(a, b))
    {
        qDebug() << idx << "unequal v1 side" << a << b;
        rv = false;
    }
    a = QLineF(v2.above,v2.v).length();
    b = QLineF(v2.v,v2.below).length();
    if (!Loose::equals(a, b))
    {
        qDebug() << idx << "unequal v2 side" << a << b;
        rv = false;
    }

    else if (type == EDGETYPE_CURVE || type == EDGETYPE_CHORD)
    {
        a = QLineF(v1.above,arcCenter).length();
        b = QLineF(v2.below,arcCenter).length();
        if (!Loose::equals(a, b))
        {
            qDebug() << idx << "unequal outer radius" << a << b;
            rv = false;
        }
        a = QLineF(v2.above,arcCenter).length();
        b = QLineF(v1.below,arcCenter).length();
        if (!Loose::equals(a, b))
        {
            qDebug() << idx << "unequal inner radius" << a << b;
            rv = false;
        }
    }
#endif
    if (rv == true)
        qDebug() << idx << "good bae";
    else
        qWarning() << idx << "bad bae";
    return rv;
}

bool BelowAndAbovePoint::validate()
{
    if (!Geo::isValid(above))
        return false;
    if (!Geo::isValid(below))
        return false;
    return true;
}

