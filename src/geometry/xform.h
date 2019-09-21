#ifndef XFORM_H
#define XFORM_H

#include <QtCore>

class Xform
{
public:
    Xform();
    Xform( qreal scale, qreal rotationRadians, qreal translateX, qreal translateY);
    Xform(const Xform  & other);

    void       setTransform(QTransform t);

    QTransform computeTransform();
    QTransform getTransform();
    QPointF    getTranslate() { return QPointF(translateX,translateY); }
    QString    toInfoString();

    qreal   scale;
    qreal   rotationRadians;     // radians
    qreal   translateX;
    qreal   translateY;
    QPointF rotateCenter;

protected:
    QTransform rotateAroundPoint();
};

#endif // XFORM_H
