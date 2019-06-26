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
    *this = scale(1.0);
}

Transform::Transform( qreal a, qreal b, qreal c, qreal d, qreal e, qreal f )
{
    this->a = a; this->b = b; this->c = c;
    this->d = d; this->e = e; this->f = f;
}

Transform::Transform( qreal ds[] )
{
    this->a = ds[0]; this->b = ds[1]; this->c = ds[2];
    this->d = ds[2]; this->e = ds[4]; this->f = ds[5];
}

Transform::Transform(QTransform qt)
{
    a = qt.m11();
    d = qt.m12();
    b = qt.m13();
    e = qt.m21();
    c = qt.m22();
    f = qt.m23();
}

Transform Transform::clone() const
{
    return Transform( a, b, c, d, e, f );
}

void Transform::assign( Transform other )
{
    this->a = other.a; this->b = other.b; this->c = other.c;
    this->d = other.d; this->e = other.e; this->f = other.f;
}

// Access.

void Transform::get( qreal * ds )
{
    ds[0] = a;
    ds[1] = b;
    ds[2] = c;
    ds[3] = d;
    ds[4] = e;
    ds[5] = f;
}

// Methods to build some common transforms.

Transform Transform::scale( qreal r )
{
    return Transform(
                r, 0, 0,
                0, r, 0 );
}

Transform Transform::scale( qreal xs, qreal ys )
{
    return Transform(
                xs, 0,  0,
                0,  ys, 0 );
}

Transform Transform::translate( qreal x, qreal y )
{
    return Transform(
                1.0, 0, x,
                0, 1.0, y );
}

Transform Transform::translate( QPointF pt )
{
    return Transform(
                1.0, 0, pt.x(),
                0, 1.0, pt.y() );
}

Transform Transform::rotate( qreal t )
{
    return Transform(
                qCos( t ), -qSin( t ), 0,
                qSin( t ),  qCos( t ), 0 );
}

Transform Transform::rotateAroundPoint( QPointF pt, qreal t )
{
    return (Transform::translate( pt ).compose(
                Transform::rotate( t ).compose(
                    Transform::translate( -pt.x(), -pt.y() ) ) ));
}

Transform Transform::reflectThroughLine( QPointF p, QPointF q )
{
    Transform T = matchLineSegment( p, q );
    return T.compose(
                Transform::scale( 1.0, -1.0 ).compose( T.invert() ) );
}

QTransform Transform::getQTransform()
{
    QTransform t(a,d,b,e,c,f,0,0);
    return t;
}

QMatrix Transform::getQMatrix()
{
    QMatrix m(a,d,b,e,c,f);
    return m;
}

// Matrix multiplication.

Transform Transform::compose(const Transform & other )
{
    return Transform(
                a * other.a + b * other.d,
                a * other.b + b * other.e,
                a * other.c + b * other.f + c,

                d * other.a + e * other.d,
                d * other.b + e * other.e,
                d * other.c + e * other.f + f );
}

void Transform::composeD( Transform & other )
{
    qreal na = a * other.a + b * other.d;
    qreal nb = a * other.b + b * other.e;
    qreal nc = a * other.c + b * other.f + c;
    qreal nd = d * other.a + e * other.d;
    qreal ne = d * other.b + e * other.e;
    qreal nf = d * other.c + e * other.f + f;

    a = na; b = nb; c = nc;
    d = nd; e = ne; f = nf;
}

// Transforming points.

QPointF Transform::apply( QPointF v ) const
{
    qreal x = v.x();
    qreal y = v.y();

    return QPointF(
                a * x + b * y + c,
                d * x + e * y + f );
}

QPointF Transform::apply( qreal x, qreal y ) const
{
    return QPointF(
                a * x + b * y + c,
                d * x + e * y + f );
}

void Transform::applyD( QPointF & v )
{
    qreal x = v.x();
    qreal y = v.y();

    v.setX( a * x + b * y + c );
    v.setY( d * x + e * y + f );
}

// These two routines don't create any new objects since
// qreals can be moved around on the Java stack.  Therefore
// they can be a lot more efficient for mathematical computation
// than the versions above, even though two calls are involved
// instead of one.  These were added late in the game, though,
// so it's not clear they'll get used.

qreal Transform::applyX( qreal x, qreal y ) const
{
    return a * x + b * y + c;
}

qreal Transform::applyY( qreal x, qreal y ) const
{
    return d * x + e * y + f;
}

// Transforming a vector of points.

QPolygonF Transform::apply(QPolygonF & vs)
{
    int len = vs.size();

    QPolygonF ret;
    for( int idx = 0; idx < len; idx++ )
    {
        ret << apply( vs[ idx ] );
    }

    return ret;
}

void Transform::applyD(QPolygonF & vs)
{
    int len = vs.size();

    for( int idx = 0; idx < len; idx++ )
    {
        applyD( vs[ idx ] );
    }
}

QLineF Transform::apply(QLineF & line)
{
    return QLineF(apply(line.p1()),apply(line.p2()));
}

QRectF Transform::apply(QRectF & vs)
{
    QPointF tl = vs.topLeft();
    QPointF br = vs.bottomRight();
    return QRectF(apply(tl),apply(br));
}

Transform Transform::invert() const
{
    qreal det = a * e - b * d;

    if( det == 0.0 )
    {
        qFatal("Non invertible matrix.");
    }

    return Transform(
                e / det, -b / det, ( b * f - c * e ) / det,
                -d / det, a / det, ( c * d - a * f ) / det );
}

void Transform::invertD()
{
    qreal det = a * e - b * d;

    if( det == 0.0 )
    {
        qFatal("Non invertible matrix.");
    }

    qreal na = e / det;
    qreal nb = -b / det;
    qreal nc = ( b * f - c * e ) / det;
    qreal nd = -d / det;
    qreal ne = a / det;
    qreal nf = ( c * d - a * f ) / det;

    a = na; b = nb; c = nc;
    d = nd; e = ne; f = nf;
}

// Provide the transform matrix to carry the unit interval
// on the positive X axis to the line segment from p to q.

Transform Transform::matchLineSegment( QPointF p, QPointF q )
{
    qreal px = p.x();
    qreal py = p.y();
    qreal qx = q.x();
    qreal qy = q.y();

    return Transform(
                qx - px, py - qy, px,
                qy - py, qx - px, py );
}

// get the transform that carries p1->q1 to p2->q2.

Transform Transform::matchTwoSegments( QPointF p1, QPointF q1, QPointF p2, QPointF q2 )
{
    Transform top1q1 = matchLineSegment( p1, q1 );
    Transform top2q2 = matchLineSegment( p2, q2 );
    return top2q2.compose( top1q1.invert() );
}

// Get distance from zero for a qreal value.
// Used to calculate radius, jitters, etc.

qreal Transform::distFromZero( qreal v ) const
{
    QPointF a = apply(QPointF(v,0.0));
    QPointF b = apply(QPointF(0.0,0.0));
    return QLineF(a,b).length();
}

qreal Transform::distFromInvertedZero( qreal v ) const
{
    return invert().distFromZero( v );
}

// Returns true if this transform includes a reflection.

bool Transform::flips()
{
    return (a*e - b*d) < 0.0;
}

QString Transform::toString()
{
#if 0
    return
            "\n[ " + QString::number(a) + " " + QString::number(b) + " " + QString::number(c) + " ]\n" +
            "[ " + QString::number(d) + " " + QString::number(e) + " " + QString::number(f) + " ]\n" +
            + "scale=" + QString::number(scalex()) + " rot=" + QString::number(rotation())
            + " trans=" + QString::number(transx()) + " " + QString::number(transy());
            //"[ 0 0 1 ]";
#endif
    return
            "scale=" + QString::number(scalex()) + " rot=" + QString::number(rotation()) + " (" + QString::number(qRadiansToDegrees(rotation())) + ")"
            + " trans=" + QString::number(transx()) + " " + QString::number(transy());
}

// Equality.

bool Transform::equals(const Transform & other )
{
    return (a == other.a) && (b == other.b) && (c == other.c) &&
            (d == other.d) && (e == other.e) && (f == other.f);
}

int Transform::hashCode()
{
    return (qHash( a ) ^ qHash( b ) ^ qHash( c ) ^ qHash( d ) ^ qHash( e ) ^ qHash( f )) ;
}

Transform Transform::compose(qreal scale, qreal rotate, QPointF translate)
{
    // model-to-world: translate/rotate/scale
    return Transform::translate(translate).compose(Transform::rotate(rotate)).compose(Transform::scale(scale));
}

void Transform::decompose(Transform t, qreal & scale, qreal & rotate, QPointF & translate)
{
    scale     = t.scalex();
    rotate    = t.rotation();
    translate = t.trans();
}



