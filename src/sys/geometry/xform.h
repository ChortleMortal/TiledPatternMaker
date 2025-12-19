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
    bool    operator==(const Xform & other) const;
    bool    operator!=(const Xform & other) const { return !(*this == other); }

    void        setTransform(QTransform t);

    QTransform  getTransform() const;
    QTransform  getTransform(QPointF rotCenter) const;

    QPointF     getTranslate() { return QPointF(translateX,translateY); }

    qreal       getScale() const;
    qreal       getRotateRadians() const;
    qreal       getRotateDegrees() const;
    qreal       getTranslateX() const;
    qreal       getTranslateY() const;
    QPointF     getTranslate() const;

    void        setScale(qreal s);
    void        setRotateRadians(qreal rr);
    void        setRotateDegrees(qreal deg);
    void        setTranslateX(qreal x);
    void        setTranslateY(qreal y);
    void        setTranslate(QPointF t);
    void        applyTranslate(QPointF t);

    QString     info(int precision = 16) const;
    qreal       hash();

protected:
    QTransform  rotateAroundPoint(QPointF pt);

private:
    qreal       scale;
    qreal       rotRadians;     // radians
    qreal       translateX;     // screen units
    qreal       translateY;     // screen units
};

#endif // XFORM_H
