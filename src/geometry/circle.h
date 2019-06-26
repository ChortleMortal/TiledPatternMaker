#ifndef CIRCLE_H
#define CIRCLE_H

#include <QPointF>
#include "geometry/Loose.h"

class Circle;
typedef std::shared_ptr<Circle> CirclePtr;

class Circle
{
public:
    Circle();
    Circle(const Circle & c);
    Circle(QPointF centre, qreal radius);

    QRectF  boundingRect();
    qreal   x() { return centre.x(); }
    qreal   y() { return centre.y(); }
    bool    operator==(const Circle & other);

    friend QDataStream & operator<< (QDataStream &out, const Circle & c)
    {
        out << c.radius;
        out << c.centre;
        return out;
    }

    friend QDataStream & operator>> (QDataStream &in, Circle & c)
    {
        in >> c.radius;
        in >> c.centre;
        return in;
    }

    friend QDataStream & operator<< (QDataStream &out, const CirclePtr c)
    {
        out << c->radius;
        out << c->centre;
        return out;
    }

    friend QDataStream & operator>> (QDataStream &in, CirclePtr & c)
    {
        qreal radius;
        QPointF center;
        in >> radius;
        in >> center;
        c = std::make_shared<Circle>(center, radius);
        return in;
    }

    void        dump();

    qreal       radius;
    QPointF     centre;
    qreal       tmpDist2;   // variable omly used in selection
};


#endif
