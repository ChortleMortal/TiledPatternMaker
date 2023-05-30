#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

////////////////////////////////////////////////////////////////////////////
//
// Transform.java
//
// A two-dimensional affine transform.  I store the top two rows of
// the homogeneous 3x3 matrix.  Or, looked at another way, it's the
// 2D linear transform ((a b) (d e)) together with the translation (c f).

#include <QTransform>
#include <QtMath>

class Transform
{
public:
    static QTransform rotateAroundPoint(QPointF pt, qreal t);   // radians
    static QTransform rotate(qreal t);                          // radians
    static QTransform scaleAroundPoint(QPointF pt, qreal t);

    static qreal   distFromZero(QTransform t, qreal v);
    static qreal   distFromInvertedZero(QTransform t, qreal v) ;

    static qreal   scalex(QTransform T)     { return qSqrt(T.m11() * T.m11() + T.m12() * T.m12()); }
    static qreal   scaley(QTransform T)     { return qSqrt(T.m21() * T.m21() + T.m22() * T.m22()); }
    static qreal   rotation(QTransform T)   { return qAtan2(T.m12(), T.m11()); }    // radians
    static qreal   transx(QTransform T)     { return T.m31(); }
    static qreal   transy(QTransform T)     { return T.m32(); }
    static QPointF trans(QTransform T)      { return QPointF(T.m31(),T.m32()); }

    static QString toString(QTransform t);
    static QString toInfoString(QTransform t, int decimals = 6);
};

#endif

