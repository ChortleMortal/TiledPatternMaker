#include "geometry/polygon.h"
#include "tile/tile.h"

APolygon::APolygon()
{
    hasSource   = false;
    sides       = 4;
    rot         = 0;
    scale       = 1.0;
}

APolygon::APolygon(QPolygonF & poly)
{
    source      = poly;
    hasSource   = true;
    rot         = 0;
    scale       = 1.0;
}

APolygon::APolygon(int sides, qreal rotDegrees, qreal scale)
{
    this->sides = sides;
    rot         = rotDegrees;
    this->scale = scale;
    hasSource   = false;
}

APolygon::APolygon(const APolygon & other)
{
    pos         = other.pos;
    sides       = other.sides;
    rot         = other.rot;
    scale       = other.scale;
    source      = other.source;
    hasSource   = other.hasSource;
}

APolygon & APolygon::operator=(const APolygon & other)
{
    pos         = other.pos;
    sides       = other.sides;
    rot         = other.rot;
    scale       = other.scale;
    source      = other.source;
    hasSource   = other.hasSource;
    return * this;
}

QPolygonF APolygon::get()
{
    QPolygonF p;
    if (hasSource)
    {
        QTransform transform;
        transform.translate(pos.x(), pos.y());
        transform.rotate(rot);
        transform.scale(scale,scale);
        p = transform.map(source);
    }
    else
    {
        Tile atile(sides,rot,scale);
        p = atile.getPolygon();
        p = QTransform::fromTranslate(pos.x(),pos.y()).map(p);

    }
    return p;
}

