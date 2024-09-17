#include <QPainter>
#include "sys/geometry/crop.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/transform.h"
#include "model/settings/configuration.h"
#include <QDebug>

/*
    A crop is a an adjustable data structure which is used to perform
    operations on a map.

    The boundary of the crop can be embedded into a map
    The boundary of the crop can be used to delete everything outside it

    The crop can also draw itself
*/

Crop::Crop()
{
    _embed      = false;
    _apply      = false;
    _clip       = false;
    _cropType   = CROP_UNDEFINED;
    _aspect     = ASPECT_UNCONSTRAINED;
    _vAspect    = false;
}

Crop::Crop(const Crop & other)
{
    _embed      = false;
    _apply      = false;
    _clip       = false;
    _cropType   = other._cropType;
    _aspect     = other._aspect;
    _vAspect    = other._vAspect;

    _poly       = other._poly;
    _circle     = other._circle;
    _rect       = other._rect;
}

void Crop::setRect(QRectF & rect)
{
    _cropType = CROP_RECTANGLE;
    _rect = rect.normalized();
    adjust();
}

void Crop::adjust()
{
    if (_cropType != CROP_RECTANGLE)
        return;

    qreal mult;

    switch (_aspect)
    {
    case ASPECT_UNCONSTRAINED:
    default:
        return;

    case ASPECT_SQRT_2:
        mult = M_SQRT2;
        break;

    case ASPECT_SQRT_3:
        mult = qSqrt(3.0);
        break;

    case ASPECT_SQRT_4:
        mult = 2.0;
        break;

    case ASPECT_SQRT_5:
        mult = qSqrt(5.0);
        break;

    case ASPECT_SQRT_6:
        mult = qSqrt(6.0);
        break;

    case ASPECT_SQRT_7:
        mult = qSqrt(7.0);
        break;

    case ASPECT_SQUARE:
        mult = 1.0;
        break;

    case ASPECT_SD:
        mult = 4.0/3.0;
        break;

    case ASPECT_HD:
        mult = 16.0/9.0;
        break;
    }

    if (_vAspect)
    {
        _rect.setHeight(_rect.width() * mult);
    }
    else
    {
        _rect.setWidth(_rect.height() * mult);
    }
}

void Crop::setCircle(Circle &c)
{
    _circle   = c;
    _cropType = CROP_CIRCLE;
}

void Crop::setPolygon(APolygon & p)
{
    // this is a regular polygon
    _poly = p;
    _cropType = CROP_POLYGON;
}

void Crop::setPolygon(QPolygonF & p)
{
    _poly.set(p);
    _cropType = CROP_POLYGON;
}


QString Crop::getCropString()
{
    QString astring = "Undefined";
    switch(_cropType)
    {
    case CROP_RECTANGLE:
        astring = QString("Rect: %1,%2,%3x%4").arg(_rect.x()).arg(_rect.y()).arg(_rect.width()).arg(_rect.height());
        break;
    case CROP_CIRCLE:
            astring =  QString("Circle: %1 %2 %3").arg(_circle.radius).arg(_circle.centre.x()).arg(_circle.centre.y());
        break;
    case CROP_POLYGON:
            astring = "Polygon";
    default:
    case CROP_UNDEFINED:
        break;
    }
    return astring;
}

QPointF Crop::getCenter()
{
    QPointF center;
    switch (_cropType)
    {
    default:
    case CROP_UNDEFINED:
        break;
    case CROP_RECTANGLE:
        center = _rect.center();
        break;
    case CROP_CIRCLE:
            center = _circle.centre;
        break;
    case CROP_POLYGON:
        center = Geo::center(_poly.get());
        break;
    }
    return center;
}

// we need this because real crops are in model units
// but border boundaries are in screen unitw
void  Crop::transform(QTransform t)
{
    _rect = t.mapRect(_rect);

    auto p = t.map(_poly.get());
    _poly.set(p);

    _circle.centre = t.map(_circle.centre);
    _circle.radius = Transform::scalex(t) * _circle.radius;
}

void Crop::draw(QPainter * painter, QTransform t, bool active)
{
    painter->save();

    if (active)
    {
        painter->setPen(QPen(Qt::red,3));
        painter->setBrush(QBrush(0xff0000,Qt::Dense7Pattern));
    }
    else
    {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(Qt::red,3));
    }

    //qDebug() << "Crop::draw" << Transform::toInfoString(t);
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (_cropType == CROP_RECTANGLE)
    {
        QRectF rect = t.mapRect(_rect);
        painter->drawRect(rect);
    }
    else if (_cropType == CROP_POLYGON)
    {
        QPolygonF p2 = t.map(_poly.get());
        painter->drawPolygon(p2);
    }
    else if (_cropType == CROP_CIRCLE)
    {
        QPointF center = _circle.centre;
        center         = t.map(center);
        qreal radius   = _circle.radius;
        radius        *= Transform::scalex(t);
        painter->drawEllipse(center,radius,radius);
    }

    if (Sys::config->showCenterDebug)
    {
        // draw center X
        qreal len = 9.0;
        QPointF pt = getCenter();
        pt = t.map(pt);
        painter->drawLine(QPointF(pt.x()-len,pt.y()),QPointF(pt.x()+len,pt.y()));
        painter->drawLine(QPointF(pt.x(),pt.y()-len),QPointF(pt.x(),pt.y()+len));
    }

    painter->restore();
}

void Crop::dump()
{
    switch(_cropType)
    {
    case CROP_RECTANGLE:
        qDebug() << "Crop rect:" << _rect;
        break;
    case CROP_POLYGON:
        qDebug() << "Crop poly:" << _poly.get();
        break;
    case CROP_CIRCLE:
        qDebug() << "Crop: radius =" << _circle.radius;
        break;
    case CROP_UNDEFINED:
        qDebug() << "Crop: undefined";
        break;
    }
}
