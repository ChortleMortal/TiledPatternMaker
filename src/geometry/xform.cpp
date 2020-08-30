#include "xform.h"
#include "geometry/transform.h"

Xform::Xform()
{
    scale      = 1.0;
    translateX = 0.0;
    translateY = 0.0;
    rotationRadians  = 0.0;
    hasCenter = false;
}

Xform::Xform( qreal scale, qreal rotation, qreal translateX, qreal translateY)
{
    this->scale      = scale;
    this->translateX = translateX;
    this->translateY = translateY;
    this->rotationRadians = rotation;
    hasCenter = false;
}

Xform::Xform(const Xform  & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotationRadians = other.rotationRadians;
    center       = other.center;
    hasCenter    = other.hasCenter;
}

Xform::Xform(QTransform t)
{
    setTransform(t);
    hasCenter = false;
}

void Xform::init()
{
    scale           = 1.0;
    translateX      = 0.0;
    translateY      = 0.0;
    rotationRadians = 0.0;
    center          = QPointF();
}

void Xform::setTransform(QTransform t)
{
    scale           = Transform::scalex(t);
    translateX      = Transform::transx(t);
    translateY      = Transform::transy(t);
    rotationRadians = Transform::rotation(t);
}

void Xform::addTransform(QTransform t)
{
    scale           += Transform::scalex(t);
    translateX      += Transform::transx(t);
    translateY      += Transform::transy(t);
    rotationRadians += Transform::rotation(t);
}

QTransform Xform::getTransform()
{
    QTransform t;
    t.translate(translateX,translateY);
    t.rotateRadians(rotationRadians);
    t.scale(scale,scale);
    //qDebug().noquote() << "XForm::getTransform()" << Transform::toInfoString(t);
    return t;
}

QTransform Xform::toQTransform(QTransform transform)
{
    QPointF cent = transform.map(center);
    QTransform t;
    t.translate(cent.x(), cent.y());
    t.translate(translateX,translateY);
    t.rotateRadians(rotationRadians);
    t.scale(scale,scale);
    t.translate(-cent.x(), -cent.y());
    //qDebug().noquote() << "XForm::toQTransform()" << Transform::toInfoString(t);
    return t;
}

QTransform Xform::rotateAroundPoint(QPointF pt)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform().rotateRadians(rotationRadians) * QTransform().translate(pt.x(), pt.y())));
}

QTransform Xform::scaleAroundPoint(QPointF pt)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform::fromScale(scale,scale) * QTransform().translate(pt.x(), pt.y())));
}

QString Xform::toInfoString()
{
    QString s;
    s  = QString("Scale=%1 ").arg(QString::number(scale,'g',16));
    s += QString("Rot=%1 ").arg(QString::number(rotationRadians,'g',16));
    s += QString("X=%1 ").arg(QString::number(translateX,'g',16));
    s += QString("Y=%1 Center=").arg(QString::number(translateY,'g',16));
    QDebug deb(&s);
    deb << center;
    return s;
}

qreal Xform::getScale()
{
    return scale;
}

qreal Xform::getRotateRadians()
{
    return rotationRadians;
}

qreal Xform::getRotateDegrees()
{
    return qRadiansToDegrees(rotationRadians);
}

qreal Xform::getTranslateX()
{
    return translateX;
}

qreal Xform::getTranslateY()
{
    return translateY;
}

void Xform::setScale(qreal s)
{
    scale = s;
}

void Xform::setRotateRadians(qreal rr)
{
    rotationRadians = rr;
}

void Xform::setRotateDegrees(qreal deg)
{
    rotationRadians = qDegreesToRadians(deg);
}

void Xform::setTranslateX(qreal x)
{
    translateX = x;
}

void Xform::setTranslateY(qreal y)
{
    translateY = y;
}

QPointF Xform::getCenter()
{
    //qDebug().noquote() << "Xform::getCenter:" << center;
    return center;
}

void Xform::setCenter(QPointF pt)
{
    if (pt.x() > 10 || pt.y() > 10)
    {
        qWarning() << "not model units";
    }
    center    = pt;
    hasCenter = true;
    qDebug().noquote() << "Xform::setCenter:" << center;
}
