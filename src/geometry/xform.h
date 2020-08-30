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
    void       addTransform(QTransform t);

    QTransform toQTransform(QTransform transform);
    QTransform getTransform();
    QPointF    getTranslate() { return QPointF(translateX,translateY); }
    QString    toInfoString();

    qreal      getScale();
    qreal      getRotateRadians();
    qreal      getRotateDegrees();
    qreal      getTranslateX();
    qreal      getTranslateY();
    QPointF    getCenter();

    void       setScale(qreal s);
    void       setRotateRadians(qreal rr);
    void       setRotateDegrees(qreal deg);
    void       setTranslateX(qreal x);
    void       setTranslateY(qreal y);
    void       setCenter(QPointF pt);

    bool    hasCenter;

protected:
    QTransform rotateAroundPoint(QPointF pt);
    QTransform scaleAroundPoint(QPointF pt);

private:
    qreal   scale;
    qreal   rotationRadians;     // radians
    qreal   translateX;
    qreal   translateY;
    QPointF center;             // model units
};

#endif // XFORM_H
