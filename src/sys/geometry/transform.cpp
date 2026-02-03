////////////////////////////////////////////////////////////////////////////
//
// Transform.java
//
// A two-dimensional affine transform.  I store the top two rows of
// the homogeneous 3x3 matrix.  Or, looked at another way, it's the
// 2D linear transform ((a b) (d e)) together with the translation (c f).

#include "sys/geometry/transform.h"

QTransform Transform::scaleAroundPoint(QPointF pt, qreal t)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform::fromScale(t,t) * QTransform().translate(pt.x(), pt.y())));
}

QTransform Transform::rotateRadiansAroundPoint(QPointF pt, qreal t)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform().rotateRadians(t) * QTransform().translate(pt.x(), pt.y())));
}

QTransform Transform::rotateDegreesAroundPoint(QPointF pt, qreal t)
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform().rotate(t) * QTransform().translate(pt.x(), pt.y())));
}

QTransform Transform::rotate(qreal t)
{
    // t is in radians
    // qCos( t ), -qSin( t ), 0,
    // qSin( t ), qCos( t ), 0 );
    return QTransform(qCos(t),qSin(t),-qSin(t),qCos(t),0.0,0.0);
}

qreal Transform::distFromInvertedZero(QTransform t, qreal v)
{
    return distFromZero(t.inverted(),v);
}

qreal Transform::distFromZero(QTransform t, qreal v)
{
    QPointF a = t.map(QPointF(v,0.0));
    QPointF b = t.map(QPointF(0.0,0.0));
    return QLineF(a,b).length();
}

QString Transform::writeInfo(QTransform t)
{
    QString s = QString::number(t.m11(),'g',16) + "," + QString::number(t.m12(),'g',16) + "," + QString::number(t.m13(),'g',16) + ","
              + QString::number(t.m21(),'g',16) + "," + QString::number(t.m22(),'g',16) + "," + QString::number(t.m23(),'g',16) + ","
              + QString::number(t.m31(),'g',16) + "," + QString::number(t.m32(),'g',16) + "," + QString::number(t.m33(),'g',16);
    return s;
}

QString Transform::info(QTransform t, int decimals)
{
    if (!t.isAffine())
        return "Not affine!";

    QString sscale;
    QString qsx = QString::number(scalex(t),'f',decimals);
    QString qsy = QString::number(scaley(t),'f',decimals);
    if (qsx == qsy)
        sscale = qsx;
    else
        sscale = qsx + " " + qsy;

    QString s =  "scale=" + sscale + " rot=" + QString::number(qRadiansToDegrees(rotation(t)),'f',decimals)
              + " trans=" + QString::number(transx(t),'f',decimals) + " " + QString::number(transy(t),'f',decimals);
    return s;
}






