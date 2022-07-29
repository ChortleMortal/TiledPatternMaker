#include <QDebug>
#include "geometry/xform.h"
#include "geometry/transform.h"

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
    modelCenter  = other.modelCenter;
}

Xform & Xform::operator=(const Xform & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotRadians   = other.rotRadians;
    modelCenter  = other.modelCenter;
    return *this;
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
    QPointF cent = transform.map(modelCenter);
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
    t.translate(modelCenter.x(), modelCenter.y());
    t.translate(translateX,translateY);
    t.rotateRadians(rotRadians);
    t.scale(scale,scale);
    t.translate(-modelCenter.x(), -modelCenter.y());
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

QString Xform::toInfoString(int precision) const
{
    QString s;
    s  = QString("Scale=%1 ").arg(QString::number(scale,'g',precision));
    s += QString("Rot=%1 ").arg(QString::number(rotRadians,'g',precision));
    s += QString("X=%1 ").arg(QString::number(translateX,'g',precision));
    s += QString("Y=%1 Center=").arg(QString::number(translateY,'g',precision));
    QDebug deb(&s);
    deb << modelCenter;
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

void Xform::setTranslateX(qreal x)
{
    translateX = x;
}

void Xform::setTranslateY(qreal y)
{
    translateY = y;
}

void Xform::setModelCenter(QPointF mpt)
{
    modelCenter = mpt;  // model units
    qDebug() << "modelCenter:" << modelCenter;
}
