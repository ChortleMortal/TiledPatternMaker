/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////
//
// Transform.java
//
// A two-dimensional affine transform.  I store the top two rows of
// the homogeneous 3x3 matrix.  Or, looked at another way, it's the
// 2D linear transform ((a b) (d e)) together with the translation (c f).


#include "geometry/Transform.h"

Transform::Transform()
{
    a = 1.0; b=0; c=0; d=0; e=1.0; f=0;
}

Transform::Transform( qreal a, qreal b, qreal c, qreal d, qreal e, qreal f )
{
    this->a = a; this->b = b; this->c = c;
    this->d = d; this->e = e; this->f = f;
}

Transform::Transform(QTransform t)
{
    QMatrix qt = t.toAffine();

    a = qt.m11();
    b = qt.m21();
    c = qt.dx();
    d = qt.m12();
    e = qt.m22();
    f = qt.dy();
}

QVector<qreal> Transform::get()
{
    QVector<qreal> ds;
    ds << a;
    ds << b;
    ds << c;
    ds << d;
    ds << e;
    ds << f;
    return ds;
}

QTransform Transform::getQTransform()
{
    QMatrix qm = getQMatrix();
    return QTransform(qm);
}

QMatrix Transform::getQMatrix()
{
    QMatrix m(a,d,
              b,e,
              c,f);
    return m;
}

QTransform Transform::rotateAroundPoint( QPointF pt, qreal t )
{
    return (QTransform::fromTranslate(-pt.x(),-pt.y()) * (QTransform().rotateRadians(t) * QTransform().translate(pt.x(), pt.y())));
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

QString Transform::toString(QTransform t)
{
    QString s = QString::number(t.m11(),'g',16) + "," + QString::number(t.m12(),'g',16) + "," + QString::number(t.m13(),'g',16) + ","
              + QString::number(t.m21(),'g',16) + "," + QString::number(t.m22(),'g',16) + "," + QString::number(t.m23(),'g',16) + ","
              + QString::number(t.m31(),'g',16) + "," + QString::number(t.m32(),'g',16) + "," + QString::number(t.m33(),'g',16);
    return s;
}

QString Transform::toInfoString(QTransform t)
{
    QString s =  "scale=" + QString::number(scalex(t)) + " rot=" + QString::number(rotation(t)) + " (" + QString::number(qRadiansToDegrees(rotation(t))) + ")"
              + " trans=" + QString::number(transx(t)) + " " + QString::number(transy(t));
    return s;
}






