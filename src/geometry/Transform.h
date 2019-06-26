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
#include <QHash>
#include <QTransform>
#include <QMatrix>

#define M11 a
#define M21 b
#define M31 c
#define M12 d
#define M22 e
#define M32 f


class Transform
{
public:
    Transform();
    Transform( qreal a, qreal b, qreal c, qreal d, qreal e, qreal f );
    Transform( qreal ds[] );
    Transform(QTransform qt);

    Transform clone() const;

    void assign( Transform other );

    void get( qreal * ds );

    static Transform scale( qreal r );
    static Transform scale( qreal xs, qreal ys );
    static Transform translate( qreal x, qreal y );
    static Transform translate( QPointF pt );
    static Transform rotate( qreal t );
    static Transform rotateAroundPoint( QPointF pt, qreal t );
    static Transform reflectThroughLine( QPointF p, QPointF q );

    QTransform getQTransform();
    QMatrix    getQMatrix();

    Transform compose(const Transform & other );
    void      composeD( Transform & other );

    static Transform compose(qreal scale, qreal rotate, QPointF translate);
    static void decompose(Transform t, qreal & scale, qreal & rotate, QPointF & translate);

    qreal       applyX( qreal x, qreal y ) const;
    qreal       applyY( qreal x, qreal y ) const;

    QPointF     apply(QPointF v) const;
    QPointF     apply(qreal x, qreal y) const;
    QRectF      apply(QRectF & vs );
    QLineF      apply(QLineF & line);
    QPolygonF   apply(QPolygonF & vs);

    void        applyD(QPointF & v);
    void        applyD(QPolygonF & vs);

    Transform   invert() const;
    void        invertD();

    static Transform matchLineSegment( QPointF p, QPointF q );
    static Transform matchTwoSegments( QPointF p1, QPointF q1, QPointF p2, QPointF q2 );

    qreal distFromZero( qreal v ) const;
    qreal distFromInvertedZero( qreal v ) const;

    bool flips();

    QString toString();

    bool equals(const Transform & other );
    int hashCode();

    qreal scalex() const  { return sqrt(M11 * M11 + M12 * M12); }
    qreal scaley()   { return sqrt(M21 * M21 + M22 * M22); }
    qreal rotation() { return atan2(M12, M11);}
    qreal transx()   { return M31;}
    qreal transy()   { return M32;}
    QPointF trans()  { return QPointF(transx(),transy()); }

private:
    qreal a, b, c;
    qreal d, e, f;

};

typedef std::shared_ptr<Transform> TransformPtr;

#endif

