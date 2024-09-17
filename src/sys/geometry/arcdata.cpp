#include "sys/geometry/arcdata.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/vertex.h"

ArcData::ArcData()
{
    _arcType    = AT_UNDEFINED;
    start       = 0;
    end         = 0;
    magnitude   = 1.0;
    trace       = 0;
    setSpan(0);
}

ArcData & ArcData::operator=(const ArcData & other)
{
    _arcType        = other._arcType;
    _convexCenter   = other._convexCenter;
    _concaveCenter  = other._concaveCenter;
    _span           = other._span;
    start           = other.start;
    end             = other.end;
    magnitude       = other.magnitude;
    rect            = other.rect;
    return *this;
}

void ArcData::create(QPointF p1, QPointF p2, QPointF center, bool convex)
{
    setConvex(convex,p1,p1);

    if (convex)
        _convexCenter = center;
    else
        _concaveCenter = center;

    calcSpan(p1,p2);
}

void  ArcData::setConvex(bool convex, QPointF p1, QPointF p2)
{
    eArcType at = (convex) ? AT_CONVEX : AT_CONCAVE;
    if (_arcType == at)
    {
        return;
    }

    if (_arcType == AT_UNDEFINED)
    {
        _arcType = at;
        return;
    }

    if (at == AT_CONVEX)
    {
        // flip center for concave
        _convexCenter =  Geo::reflectPoint(getCenter(),QLineF(p1,p2));
    }
    else
    {
        // flip center for concave
        _concaveCenter =  Geo::reflectPoint(getCenter(),QLineF(p1,p2));
    }

    _arcType = at;
}

QPointF ArcData::getCenter() const
{
    if (convex())
        return _convexCenter;
    else
        return _concaveCenter;
}

void ArcData::setCenter(QPointF mpt)
{
    if (convex())
        _convexCenter = mpt;
    else
        _concaveCenter = mpt;
}

void ArcData::calcSpan(QPointF p1, QPointF p2)
{

    if (convex())
    {
        calcConvexSpan(p1,p2);
    }
    else
    {
        calcConcaveSpan(p1,p2);
    }
}

void ArcData::calcSpan(Edge * edge)
{
    QPointF p1 = edge->v1->pt;
    QPointF p2 = edge->v2->pt;

    Q_ASSERT(_arcType != AT_UNDEFINED);
    calcSpan(p1,p2);
}

void ArcData::calcConvexSpan(QPointF p1, QPointF p2)
{
    QPointF center = getCenter();

    if (trace >=3) qDebug()  << "calcConvex " << p1 << p2 << center;

    qreal radius = QLineF(p1,center).length();
    rect         = QRectF(center.x() - radius, center.y() - radius, radius*2, radius*2);
    start        = Geo::getAngleDegrees(center,p1);
    end          = Geo::getAngleDegrees(center,p2);

    qreal span   = end-start;         // should be a -ve span  (meaning CCW)

    if (trace >= 2) qDebug() << "convex start" << start << "end" << end << "span" << span;

    if (span > 0)
    {
        span = -(start + (360-end));
    }
    if (span < -180.0)
    {
        span = -(360 + span); // not tested
    }

    if (trace >= 1) dump();

    Q_ASSERT(span <= 0);

    setSpan(span);

    //qInfo() << "convex span" << getSpan();
}

void ArcData::calcConcaveSpan(QPointF p1, QPointF p2)
{
    // for concave we need the center reflection accross p1p2
    // and the direction must be CCW - meaning a positsive span

    QPointF center = getCenter();

    if (trace >=3) qDebug() << "calcConcave" << p1 << p2 << center;

    //arcCenter = Geo::reflectPoint(arcCenter,QLineF(p1,p2));   // flip center for concave

    qreal radius = QLineF(p1,center).length();
    rect    = QRectF(center.x() - radius, center.y() - radius, radius*2, radius*2);
    start   = Geo::getAngleDegrees(center,p1);
    end     = Geo::getAngleDegrees(center,p2);

    qreal span = end-start; // should be a +ve span (meaning CW)

    if (trace >= 2) qDebug() << "concave start" << start << "end" << end << "span" << span;

    if (span < 0)
    {
        span += 360;
    }

    if (span > 180.0)
    {
        span  = 360.0 - span;   // fixes FirstCurve.v4 and petal
    }

    if (trace >= 1) dump();
    Q_ASSERT(span >= 0);

    setSpan(span);

    //qInfo() << "concave span" << getSpan();
}

QString ArcData::info()
{
    QString astring;
    QDebug  deb(&astring);

    deb << "ArcData" << ((convex()) ? "convex" :  "concave") << rect << "start=" << start << "end=" << end  << "span=" << span();

    return astring;
}

void ArcData::dump()
{
    qDebug().noquote() << info();
}

QPointF ArcData::reflectCentreOverEdge(QPointF p1, QPointF p2, QPointF arcCenter)
{
    QLineF l(p1,p2);
    QPointF c3= Geo::reflectPoint(arcCenter,l);
    return c3;
}

bool ArcData::pointWithinArc(QPointF pt)
{
    qreal angle =  Geo::getAngleDegrees(getCenter(),pt);

    if (Loose::equals(angle,start))
        return true;
    if (Loose::equals(angle,end))
        return true;

    //qInfo() << "raw" << start << end << angle << (isConvex() ? "convex" : "concave");

    // polys are clockwise - angle increments anti-clockwise
    if (convex())
    {
        Q_ASSERT(_span <= 0);
        // start should be greater than end
        if (start > end)
        {
            if (angle > end && angle < start)
            {
                //qWarning() << " convex hit" << start << end << angle;
                return true;
            }
        }
        else
        {
            // wraparound condition
            if (angle < start && angle >=0)
            {
                //qInfo() << "convex wraparound HIT" << start << end << angle;
                return true;
            }
            else if (angle <= 360 && angle > end)
            {
                //qInfo() << "convex wraparound HIT" << start << end << angle;
                return true;
            }
            else
            {
                //qInfo() << "convex wraparound MISS" << start << end << angle;
                return false;
            }
        }
    }
    else
    {
        // concave - start should be less than end
        Q_ASSERT(_span >=0);
        if (start < end)
        {
            if (angle < end && angle > start)
            {
                //qWarning() << " concave hit 1" << start << end << angle;
                return true;
            }
        }
        else
        {
            // wraparound condition
            if (angle > start && angle <= 300)
            {
                //qInfo() << "concave wraparound HIT 2" << start << end << angle;
                return true;
            }
            else if (angle >= 0 && angle < end)
            {
                //qInfo() << "concave wraparound HIT 3" << start << end << angle;
                return true;
            }
            else
            {
                //qInfo() << "concave wraparound MISS 4" << start << end << angle;
                return false;
            }
        }
    }
    //qDebug() << "miss";
    return false;
}
