#ifndef XFORM_H
#define XFORM_H

#include <QtCore>

class Xform
{
public:
    Xform();
    Xform( qreal scale, qreal rotationRadians, qreal translateX, qreal translateY);
    Xform(const Xform  & other);
    Xform(QTransform t);

    void       init();
    void       setTransform(QTransform t);

    QTransform computeTransform();
    QTransform getTransform();
    QPointF    getTranslate() { return QPointF(translateX,translateY); }
    QString    toInfoString();

    qreal      getScale();
    qreal      getRotateRadians();
    qreal      getTranslateX();
    qreal      getTranslateY();
    QPointF    getRotateCenter();

    void       setScale(qreal s);
    void       setRotateRadians(qreal rr);
    void       setTranslateX(qreal x);
    void       setTranslateY(qreal y);
    void       setRotateCenter(QPointF pt);


protected:
    QTransform rotateAroundPoint();

private:
    qreal   scale;
    qreal   rotationRadians;     // radians
    qreal   translateX;
    qreal   translateY;
    QPointF rotateCenter;

};

#endif // XFORM_H
