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

    void        update(const Xform & other);
    void        setTransform(QTransform t);
    void        addTransform(QTransform t);

    QTransform  toQTransform(QTransform transform) const;
    QTransform  getTransform() const;
    QPointF     getTranslate() { return QPointF(translateX,translateY); }
    QString     toInfoString();

    qreal       getScale();
    qreal       getRotateRadians();
    qreal       getRotateDegrees() const;
    qreal       getTranslateX();
    qreal       getTranslateY();
    QPointF     getCenter() const;

    void        setScale(qreal s);
    void        setRotateRadians(qreal rr);
    void        setRotateDegrees(qreal deg);
    void        setTranslateX(qreal x);
    void        setTranslateY(qreal y);
    void        setCenter(QPointF mpt);  // model units

protected:
    QTransform  rotateAroundPoint(QPointF pt);
    QTransform  scaleAroundPoint(QPointF pt);

private:
    qreal       scale;
    qreal       rotRadians;     // radians
    qreal       translateX;
    qreal       translateY;
    QPointF     mCenter;             // model units so (0,0) is valid
};

#endif // XFORM_H
