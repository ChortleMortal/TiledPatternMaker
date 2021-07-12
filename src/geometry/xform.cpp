#include "geometry/xform.h"
#include "geometry/transform.h"

Xform::Xform()
{
    scale      = 1.0;
    translateX = 0.0;
    translateY = 0.0;
    rotRadians  = 0.0;
}

Xform::Xform( qreal scale, qreal rotation, qreal translateX, qreal translateY)
{
    this->scale      = scale;
    this->translateX = translateX;
    this->translateY = translateY;
    this->rotRadians = rotation;
}

Xform::Xform(const Xform  & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotRadians   = other.rotRadians;
    center       = other.center;
}

Xform & Xform::operator=(const Xform & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotRadians   = other.rotRadians;
    center       = other.center;
    return *this;
}

Xform::Xform(QTransform t)
{
    setTransform(t);
}

void Xform::update(const Xform  & other)
{
    // does nothing to center
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotRadians   = other.rotRadians;
}

void Xform::setTransform(QTransform t)
{
    scale       = Transform::scalex(t);
    translateX  = Transform::transx(t);
    translateY  = Transform::transy(t);
    rotRadians  = Transform::rotation(t);
}

void Xform::addTransform(QTransform t)
{
    scale       += Transform::scalex(t);
    translateX  += Transform::transx(t);
    translateY  += Transform::transy(t);
    rotRadians  += Transform::rotation(t);
}

QTransform Xform::getTransform() const
{
    QTransform t;
    t.translate(translateX,translateY);
    t.rotateRadians(rotRadians);
    t.scale(scale,scale);
    //qDebug().noquote() << "XForm::getTransform()" << Transform::toInfoString(t);
    return t;
}

QTransform Xform::toQTransform(QTransform transform) const
{
    QPointF cent = transform.map(center);
    return toQTransform(cent);
}

QTransform Xform::toQTransform(QPointF rotCenter) const
{
    QTransform t;
    t.translate(rotCenter.x(), rotCenter.y());
    t.translate(translateX,translateY);
    t.rotateRadians(rotRadians);
    t.scale(scale,scale);
    t.translate(-rotCenter.x(), -rotCenter.y());
    //qDebug().noquote() << "Xform::toQTransform()" << Transform::toInfoString(t) << rotCenter;
    return t;
}

QTransform Xform::toQTransform() const
{
    QTransform t;
    t.translate(center.x(), center.y());
    t.translate(translateX,translateY);
    t.rotateRadians(rotRadians);
    t.scale(scale,scale);
    t.translate(-center.x(), -center.y());
    //qDebug().noquote() << "Xform::toQTransform()" << Transform::toInfoString(t) << center;
    return t;
}


QTransform Xform::rotateAroundPoint(QPointF pt)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform().rotateRadians(rotRadians) * QTransform().translate(pt.x(), pt.y())));
}

QTransform Xform::scaleAroundPoint(QPointF pt)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform::fromScale(scale,scale) * QTransform().translate(pt.x(), pt.y())));
}

QString Xform::toInfoString() const
{
    QString s;
    s  = QString("Scale=%1 ").arg(QString::number(scale,'g',16));
    s += QString("Rot=%1 ").arg(QString::number(rotRadians,'g',16));
    s += QString("X=%1 ").arg(QString::number(translateX,'g',16));
    s += QString("Y=%1 Center=").arg(QString::number(translateY,'g',16));
    QDebug deb(&s);
    deb << center;
    return s;
}

qreal Xform::getScale() const
{
    return scale;
}

qreal Xform::getRotateRadians() const
{
    return rotRadians;
}

qreal Xform::getRotateDegrees() const
{
    return qRadiansToDegrees(rotRadians);
}

qreal Xform::getTranslateX() const
{
    return translateX;
}

qreal Xform::getTranslateY() const
{
    return translateY;
}

void Xform::setScale(qreal s)
{
    scale = s;
}

void Xform::setRotateRadians(qreal rr)
{
    rotRadians = rr;
}

void Xform::setRotateDegrees(qreal deg)
{
    rotRadians = qDegreesToRadians(deg);
}

void Xform::setTranslateX(qreal x)
{
    translateX = x;
}

void Xform::setTranslateY(qreal y)
{
    translateY = y;
}
