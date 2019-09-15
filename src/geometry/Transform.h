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

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <QtCore>
#include <QPolygonF>
#include <QTransform>
#include <QMatrix>

class Transform
{
public:
    Transform();
    Transform(qreal a, qreal b, qreal c, qreal d, qreal e, qreal f);
    Transform(QTransform qt);

    QVector<qreal>  get();
    QTransform      getQTransform();
    QMatrix         getQMatrix();

    static QTransform rotateAroundPoint(QPointF pt, qreal t);

    static qreal   distFromZero(QTransform t, qreal v);
    static qreal   distFromInvertedZero(QTransform t, qreal v) ;

    static qreal   scalex(QTransform T)     { return sqrt(T.m11() * T.m11() + T.m12() * T.m12()); }
    static qreal   scaley(QTransform T)     { return sqrt(T.m21() * T.m21() + T.m22() * T.m22()); }
    static qreal   rotation(QTransform T)   { return atan2(T.m12(), T.m11()); }
    static qreal   transx(QTransform T)     { return T.m31(); }
    static qreal   transy(QTransform T)     { return T.m32(); }
    static QPointF trans(QTransform T)      { return QPointF(transx(T),transy(T)); }

    static QString toString(QTransform t);
    static QString toInfoString(QTransform t);

private:
    qreal a, b, c;
    qreal d, e, f;
};

#endif

