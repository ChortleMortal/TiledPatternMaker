#include <QTransform>
#include "sys/geometry/arcdata.h"
#include "sys/geometry/geo.h"

ArcData::ArcData()
{
    _start       = 0;
    _end         = 0;
    _span       = 0;
    _magnitude  = 1.0;
    _curveType  = CURVE_UNDEFINED;
}

ArcData::ArcData(QLineF line, QPointF center, eCurveType ctype)
{
    _line       = line;
    _curveType = ctype;
    create(center);

}
ArcData & ArcData::operator=(const ArcData & other)
{
    _line           = other._line;
    _curveType      = other._curveType;
    _convexCenter   = other._convexCenter;
    _concaveCenter  = other._concaveCenter;
    _magnitude      = other._magnitude;

    create();

    return *this;
}

void ArcData::create(QLineF line, QPointF center, eCurveType ctype)
{
    _line       = line;
    _curveType = ctype;
    create(center);
}

void ArcData::create(QPointF center)
{
    if (_curveType == CURVE_CONVEX)
    {
        _convexCenter  = center;
        _concaveCenter =  Geo::reflectPoint(center,_line);
        setConvex();
    }
    else
    {
        _concaveCenter = center;
        _convexCenter  =  Geo::reflectPoint(center,_line);
        setConcave();
    }
    calcMagnitude();
}

void ArcData::create()
{
    if (_curveType == CURVE_CONVEX)
    {
        setConvex();
    }
    else
    {
        setConcave();
    }
    calcMagnitude();
}

ArcData ArcData::transform(QTransform t)
{
    QLineF line    = t.map(_line);
    QPointF center = t.map(getCenter());
    return ArcData(line,center,_curveType);
}

void ArcData::changeCurveType(eCurveType newType)
{
    Q_ASSERT(_curveType != CURVE_UNDEFINED);

    if (_curveType == newType)
    {
        return;
    }

    _curveType = newType;

    if (newType == CURVE_CONVEX)
    {
        setConvex();
    }
    else
    {
        setConcave();
    }
    calcMagnitude();
}

void ArcData::setConvex()
{
    //qDebug()  << "_setConvex -BEGIN" << _line << _convexCenter;

    qreal r = radius();
    _rect    = QRectF(_convexCenter.x() - r, _convexCenter.y() - r, r*2, r*2);
    _start   = Geo::getAngleDegrees(_convexCenter,_line.p1());
    _end     = Geo::getAngleDegrees(_convexCenter,_line.p2());
    _span   = _end - _start;         // should be a -ve span  (meaning CCW)

    //qDebug() << "convex start" << _start << "end" << _end << "span" << _span;

    if (_span > 0)
    {
        _span = -(_start + (360 - _end));
    }
    if (_span < -180.0)
    {
        _span = -(360 + _span); // not tested
    }

    //dump();

    Q_ASSERT(_span <= 0);

    //qDebug()  << "_setConvex -END" << _span;
}

void ArcData::setConcave()
{
    // for concave we need the center reflection accross p1p2
    // and the direction must be CCW - meaning a positsive span

    //qDebug() << "_setConcave" << _line << _concaveCenter;

    qreal r = radius();
    _rect    = QRectF(_concaveCenter.x() - r, _concaveCenter.y() - r, r*2, r*2);
    _start   = Geo::getAngleDegrees(_concaveCenter,_line.p1());
    _end     = Geo::getAngleDegrees(_concaveCenter,_line.p2());
    _span    = _end - _start; // should be a +ve span (meaning CW)

    //qDebug() << "concave start" << _start << "end" << _end << "span" << _span;

    if (_span < 0)
    {
        _span += 360;
    }

    if (_span > 180.0)
    {
        _span  = 360.0 - _span;   // fixes FirstCurve.v4 and petal
    }

    //dump();
    Q_ASSERT(_span >= 0);

    //qDebug()  << "_setConcave -END" << _span;
}

void ArcData::calcMagnitude()
{
    // calcs magnitude from arcCenter
    qreal dist  = Geo::signedDistToLine(getCenter(), _line);
    if (dist < 0 || Loose::zero(dist))
        _magnitude = 0;
    else
        _magnitude  = (dist/_line.length());

    //qDebug() << "dist"  << dist << "magnitude=" << _magnitude;
}

void ArcData::setArcMagnitude(qreal magnitude)
{
    // calcs arcCenter from magntiude
    //qDebug() << "setting  mag" << magnitude;

    QPointF mid = _line.center();
    QPointF pt;
    if (_curveType == CURVE_CONVEX)
    {
        pt = mid + QPointF(-_line.dy(), _line.dx());
    }
    else
    {
        pt = mid - QPointF(_line.dy(), -_line.dx());
    }

    QLineF perp(mid,pt);

    qreal arcCenterLen = magnitude * _line.length();
    perp.setLength(arcCenterLen);

    if (_curveType == CURVE_CONVEX)
    {
        _convexCenter = perp.p2();
        setConvex();
    }
    else
    {
        _concaveCenter = perp.p2();
        setConcave();
    }
    calcMagnitude();
    if (!Loose::equals(magnitude,_magnitude))
        qInfo() << "throttled";
}

QString ArcData::info()
{
    QString astring;
    QDebug  deb(&astring);

    deb << "ArcData" << sCurveType[_curveType] << _rect << "start=" << _start << "end=" << _end  << "span=" << _span;

    return astring;
}

void ArcData::dump()
{
    qDebug().noquote() << info();
}

bool ArcData::pointWithinArc(QPointF pt)
{
    qreal angle =  Geo::getAngleDegrees(getCenter(),pt);

    if (Loose::equals(angle,_start))
        return true;
    if (Loose::equals(angle,_end))
        return true;

    //qInfo() << "raw" << start << end << angle << (isConvex() ? "convex" : "concave");

    // polys are clockwise - angle increments anti-clockwise
    if (_curveType == CURVE_CONVEX)
    {
        Q_ASSERT(_span <= 0);
        // start should be greater than end
        if (_start > _end)
        {
            if (angle > _end && angle < _start)
            {
                //qWarning() << " convex hit" << start << end << angle;
                return true;
            }
        }
        else
        {
            // wraparound condition
            if (angle < _start && angle >=0)
            {
                //qInfo() << "convex wraparound HIT" << start << end << angle;
                return true;
            }
            else if (angle <= 360 && angle > _end)
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
        Q_ASSERT(_curveType == CURVE_CONCAVE);
        Q_ASSERT(_span >=0);
        if (_start < _end)
        {
            if (angle < _end && angle > _start)
            {
                //qWarning() << " concave hit 1" << start << end << angle;
                return true;
            }
        }
        else
        {
            // wraparound condition
            if (angle > _start && angle <= 300)
            {
                //qInfo() << "concave wraparound HIT 2" << start << end << angle;
                return true;
            }
            else if (angle >= 0 && angle < _end)
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

QPointF ArcData::computeNewPoint(QPointF orig, QPointF c, qreal span, bool moveA)
{
    // Compute original angle relative to center
    qreal thetaOrig = atan2(orig.y() - c.y(), orig.x() - c.x());

    // Adjust angle based on span
    qreal thetaNew = moveA ? (thetaOrig - span * (M_PI / 180.0)) : (thetaOrig + span * (M_PI / 180.0));

    // Compute new coordinates
    qreal r = sqrt(pow(orig.x() - c.x(), 2) + pow(orig.y() - c.y(), 2));

    QPointF newPt;
    newPt.setX(c.x() + r * cos(thetaNew));
    newPt.setY(c.y() + r * sin(thetaNew));
    return newPt;
}
