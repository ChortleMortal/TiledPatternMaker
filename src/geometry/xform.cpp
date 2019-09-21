#include "xform.h"
#include "geometry/Transform.h"
#include "base/canvas.h"

Xform::Xform()
{
    scale      = 1.0;
    translateX = 0.0;
    translateY = 0.0;
    rotationRadians  = 0.0;
}

Xform::Xform( qreal scale, qreal rotation, qreal translateX, qreal translateY)
{
    this->scale      = scale;
    this->translateX = translateX;
    this->translateY = translateY;
    this->rotationRadians = rotation;
}

Xform::Xform(const Xform  & other)
{
    scale        = other.scale;
    translateX   = other.translateX;
    translateY   = other.translateY;
    rotateCenter = other.rotateCenter;
    rotationRadians = other.rotationRadians;
}

void Xform::setTransform(QTransform t)
{
    scale      = Transform::scalex(t);
    translateX = Transform::transx(t);
    translateY = Transform::transy(t);
    rotationRadians = Transform::rotation(t);
}

QTransform Xform::getTransform()
{
    QTransform tr   = QTransform().rotateRadians(rotationRadians);
    QTransform ts   = QTransform::fromScale(scale,scale);
    QTransform tt   = QTransform().fromTranslate(translateX,translateY);

    qDebug().noquote() << "tr:" << Transform::toInfoString(tr);
    qDebug().noquote() << "ts:" << Transform::toInfoString(ts);
    qDebug().noquote() << "tt:" << Transform::toInfoString(tt);

    return tr * ts * tt;
}

QTransform Xform::computeTransform()
{
    Canvas * canvas = Canvas::getInstance();
    Scene * scene   = canvas->scene;
    if (scene)
    {
        rotateCenter    = canvas->scene->sceneRect().center();
    }
    QTransform tr   = rotateAroundPoint();
    QTransform ts   = QTransform::fromScale(scale,scale);
    QTransform tt   = QTransform().fromTranslate(translateX,translateY);

    qDebug().noquote() << "tr:" << Transform::toInfoString(tr);
    qDebug().noquote() << "ts:" << Transform::toInfoString(ts);
    qDebug().noquote() << "tt:" << Transform::toInfoString(tt);

    return tr * ts * tt;
}

QTransform Xform::rotateAroundPoint()
{
    return (QTransform::fromTranslate(-rotateCenter.x(),-rotateCenter.y()) * (QTransform().rotateRadians(rotationRadians) * QTransform().translate(rotateCenter.x(), rotateCenter.y())));
}

QString Xform::toInfoString()
{
    QString s;
    s  = QString("Scale=%1 ").arg(QString::number(scale,'g',16));
    s += QString("Rot=%1 ").arg(QString::number(rotationRadians,'g',16));
    s += QString("X=%1 ").arg(QString::number(translateX,'g',16));
    s += QString("Y=%1").arg(QString::number(translateY,'g',16));
    return s;
}

