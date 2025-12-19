#include <QDebug>
#include "sys/geometry/xform.h"
#include "sys/geometry/transform.h"

Xform::Xform()
{
    scale      = 1.0;
    translateX = 0.0;
    translateY = 0.0;
    rotRadians = 0.0;
}

Xform::Xform(const Xform  & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotRadians   = other.rotRadians;
}

Xform & Xform::operator=(const Xform & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotRadians   = other.rotRadians;
    return *this;
}

bool Xform::operator==(const Xform & other) const
{
    if (scale       != other.scale)       return false;
    if (translateX  != other.translateX)  return false;;
    if (translateY  != other.translateY)  return false;;
    if (rotRadians  != other.rotRadians)  return false;

    return true;
}

Xform::Xform(QTransform t)
{
    setTransform(t);
}

void Xform::setTransform(QTransform t)
{
    scale       = Transform::scalex(t);
    translateX  = Transform::transx(t);
    translateY  = Transform::transy(t);
    rotRadians  = Transform::rotation(t);
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

QTransform Xform::getTransform(QPointF rotCenter) const
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


QTransform Xform::rotateAroundPoint(QPointF pt)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform().rotateRadians(rotRadians) * QTransform().translate(pt.x(), pt.y())));
}

QString Xform::info(int precision) const
{
    QString s;
    s  = QString("scale=%1 ").arg(QString::number(scale,'g',precision));
    s += QString("rot=%1 ").arg(QString::number(rotRadians,'g',precision));
    s += QString("x=%1 ").arg(QString::number(translateX,'g',precision));
    s += QString("y=%1").arg(QString::number(translateY,'g',precision));
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

QPointF Xform::getTranslate() const
{
    return QPointF(translateX,translateY);
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

void Xform::setTranslate(QPointF t)
{
    translateX = t.x();
    translateY = t.y();
}

void Xform::applyTranslate(QPointF t)
{
    translateX += t.x();
    translateY += t.y();
}

void Xform::setTranslateX(qreal x)
{
    translateX = x;
}

void Xform::setTranslateY(qreal y)
{
    translateY = y;
}


qreal Xform::hash()
{
    qreal h = (scale +1) * (rotRadians+2) * (translateX+3) * (translateY+4);
    return h;
}
