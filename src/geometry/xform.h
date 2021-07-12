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

    Xform & operator=(const Xform & other);

    void        update(const Xform & other);
    void        setTransform(QTransform t);
    void        addTransform(QTransform t);

    QTransform  toQTransform(QTransform transform) const;
    QTransform  toQTransform(QPointF rotCenter) const;
    QTransform  toQTransform() const;

    QTransform  getTransform() const;
    QPointF     getTranslate() { return QPointF(translateX,translateY); }
    QString     toInfoString() const;

    qreal       getScale() const;
    qreal       getRotateRadians() const;
    qreal       getRotateDegrees() const;
    qreal       getTranslateX() const;
    qreal       getTranslateY() const;
    QPointF     getCenter() const { return center; }

    void        setScale(qreal s);
    void        setRotateRadians(qreal rr);
    void        setRotateDegrees(qreal deg);
    void        setTranslateX(qreal x);
    void        setTranslateY(qreal y);
    void        setCenter(QPointF mpt) { center = mpt; }  // model units

protected:
    QTransform  rotateAroundPoint(QPointF pt);
    QTransform  scaleAroundPoint(QPointF pt);

private:
    qreal       scale;
    qreal       rotRadians;     // radians
    qreal       translateX;
    qreal       translateY;
    QPointF     center;         // model units so (0,0) is valid
};

#endif // XFORM_H
