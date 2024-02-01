#pragma once
#ifndef XFORM_H
#define XFORM_H

#include <QTransform>

class Xform
{
public:
    Xform();
    Xform(const Xform  & other);
    Xform(QTransform t);

    Xform & operator=(const Xform & other);

    void        setTransform(QTransform t);
    void        addTransform(QTransform t);

    QTransform  toQTransform(QTransform transform) const;
    QTransform  toQTransform(QPointF rotCenter) const;
    QTransform  toQTransform() const;

    QTransform  getTransform() const;
    QPointF     getTranslate() { return QPointF(translateX,translateY); }
    QString     toInfoString(int precision = 16) const;

    qreal       getScale() const;
    qreal       getRotateRadians() const;
    qreal       getRotateDegrees() const;
    qreal       getTranslateX() const;
    qreal       getTranslateY() const;
    QPointF     getTranslate() const;
    QPointF     getModelCenter() const { return modelCenter; }

    void        setScale(qreal s);
    void        setRotateRadians(qreal rr);
    void        setRotateDegrees(qreal deg);
    void        setTranslate(QPointF t);
    void        setTranslateX(qreal x);
    void        setTranslateY(qreal y);
    void        setModelCenter(QPointF mpt);     // model units

protected:
    QTransform  rotateAroundPoint(QPointF pt);
    QTransform  scaleAroundPoint(QPointF pt);

private:
    qreal       scale;
    qreal       rotRadians;     // radians
    qreal       translateX;     // screen units
    qreal       translateY;     // screen units
    QPointF     modelCenter;    // model units so (0,0) is valid
};

#endif // XFORM_H
