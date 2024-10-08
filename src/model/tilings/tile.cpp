////////////////////////////////////////////////////////////////////////////
//
// Feature.java
//
// A Tile is an element of a tiling, i.e. a tile.  It's really
// just an array of points.  I could just use a geometry.Polygon,
// but I keep adding (and removing) per-point information, which makes it
// useful to store the array explicitly -- sometimes I switch Point
// for a FancyPoint of some kind.  Also, we don't expect the number
// of points to change once the tile is created, so the array
// is fine.
//
// We also store whether the Tile was created as a regular polygon.
// This helps later when deciding what Tiles can have Rosettes
// in them.

#include <QtMath>
#include <QTextStream>

#include "model/makers/tiling_maker.h"
#include "sys/geometry/edge.h"
#include "sys/geometry/geo.h"
#include "sys/geometry/loose.h"
#include "sys/geometry/vertex.h"
#include "model/tilings/tile.h"

int Tile::refs = 0;

using std::make_shared;

Tile::Tile(EdgePoly ep, qreal rotate, qreal scale)
{
    // createas irregular tile
    base        = ep;
    n           = ep.size();
    rotation    = rotate;
    this->scale = scale;
    regular    = false;
    refs++;

    compose();
}

Tile::Tile(int n, qreal rotate, qreal scale)
{
    // Create an n-sided regular polygon with a vertex at (1,0).
    this->n     = n;
    rotation    = rotate;
    this->scale = scale;
    regular    = true;
    refs++;

    createRegularBase();
    compose();
}


Tile::Tile(const TilePtr other )
{
    n           = other->n;
    rotation    = other->rotation;
    scale       = other->scale;
    regular     = other->regular;
    base        = other->base;
    epoly       = other->epoly;

    refs++;
}

Tile::Tile(const Tile & other ) : QObject(this)
{
    n           = other.n;
    rotation    = other.rotation;
    scale       = other.scale;
    regular     = other.regular;
    base        = other.base;
    epoly       = other.epoly;

    refs++;
}

Tile::~Tile()
{
    refs--;
}

bool Tile::operator == (const Tile & other) const
{
    if (n        != other.n)
        return false;
    if (rotation != other.rotation)
        return false;
    if (scale    != other.scale)
        return false;
    if (regular  != other.regular)
        return false;
    if (base     != other.base)
        return false;
    if (epoly    != other.epoly)
        return false;;
    return true;
}

void Tile::compose()
{
    epoly.clear();
    epoly = base.recreate();

    if (!Loose::zero(rotation))
        epoly.rotate(rotation);
    if (!Loose::equals(scale,1.0))
        epoly.scale(scale);
}

void Tile::decompose()
{
    base.clear();
    base = epoly.recreate();

    if (!Loose::zero(rotation))
        base.rotate(-rotation);
    if (!Loose::equals(scale,1.0))
        base.scale(1.0/scale);
}

TilePtr Tile::recreate()
{
    TilePtr f;
    if (regular)
        f = make_shared<Tile>(n,rotation,scale);
    else
        f = make_shared<Tile>(base,rotation,scale);
    return  f;
}

TilePtr Tile::copy()
{
    if (isRegular())
    {
        TilePtr fp = make_shared<Tile>(n,rotation,scale);
        return fp;
    }
    else
    {
        EdgePoly ep = base.recreate();
        TilePtr fp = make_shared<Tile>(ep,rotation,scale);
        return fp;
    }
}

void Tile::setN(int n)
{
    this->n = n;
    if (regular)
    {
        createRegularBase();
        compose();
    }
    emit sig_tileChanged();
}

void Tile::setRotation(qreal rotate)
{
    rotation = rotate;
    compose();
    emit sig_tileChanged();
}

void Tile::setScale(qreal scale)
{
    this->scale = scale;
    compose();
    emit sig_tileChanged();
}

void Tile::deltaScale(qreal delta)
{
    setScale(scale + delta);
}

void Tile::deltaRotation(qreal delta)
{
    setRotation(rotation + delta);
}

void Tile::setRegular(bool enb)
{
    if (enb == regular)
        return;

    if (enb)
    {
        regular =  true;
        createRegularBase();
        compose();
    }
    else
    {
        regular = false;
        compose();
    }
    emit sig_tileChanged();
}

void Tile::flipRegularity()
{
    if (regular)
    {
        // converting regular to irregular
        if (conversion.converted && !conversion.wasRegular)
        {
            setRegular(false);
            epoly    = conversion.ep;
            rotation = conversion.rotate;
            scale    = conversion.scale;
            conversion.converted = false;
        }
        else
        {
            conversion.rotate     = rotation;
            conversion.scale      = scale;
            conversion.wasRegular = true;
            conversion.converted  = true;
            setRegular(false);
            rotation = 0.0;
            scale    = 1.0;
        }
    }
    else
    {
        //converting irregular to to regular
        if (conversion.converted && conversion.wasRegular)
        {
            rotation = conversion.rotate;
            scale    = conversion.scale;
            conversion.converted = false;
            setRegular(true);
        }
        else
        {
            conversion.rotate     = rotation;
            conversion.scale      = scale;
            conversion.ep         = epoly;
            conversion.wasRegular = false;
            conversion.converted  = true;
            setRegular(true);
        }
    }
    emit sig_tileChanged();
}

void Tile::createRegularBase()
{
    QPolygonF p = Geo::getCircumscribedPolygon(n);
    base = EdgePoly(p);
}

bool Tile::equals(const TilePtr other)
{
    if (regular != other->regular)
        return false;
    if (!base.equals(other->base))
        return false;
    if (!Loose::equals(rotation, other->rotation))
        return false;
    if (!Loose::equals(scale, other->scale))
        return false;
    return true;
}

bool Tile::isSimilar(const TilePtr other)
{
    if (regular != other->regular)
        return false;
    if (numSides() != other->numSides())
        return false;
    if (!Loose::equals(rotation, other->rotation))
        return false;
    if (!Loose::equals(scale, other->scale))
        return false;
    return true;
}

QString Tile::toString() const
{
    QString text;
    QTextStream str(&text);
    int i=0;

    str << "{ ";
    for (auto & edge : std::as_const(epoly))
    {
        str << i+1 << ":" << edge->v1->pt.x() << " " << edge->v1->pt.y() << " , ";
        i++;
    }
    str << "}";
    return text;
}

QString Tile::toBaseString() const
{
    QString text;
    QTextStream str(&text);

    str << "{ ";
    for (auto & edge : std::as_const(base))
    {
        str << "point: " << edge->v1->pt.x() << " " << edge->v1->pt.y() << " , ";
    }
    str << "}";

    return text;
}

QString Tile::info()
{
    QString text;
    QTextStream str(&text);

    //str << "(" <<Utils::addr(this) << ") ";
    str << "sides:" << epoly.size();
    if (regular)
        str << " regular";
    else
        str << " irregular";

    return text;
}

QString Tile::summary()
{
    return QString("%1%2 ").arg(numSides()).arg((regular) ? 'r' : 'i' );
}

QPointF Tile::getCenter()
{
    if (regular)
        return epoly.calcCenter();
    else
        return epoly.calcIrregularCenter();
}

QLineF Tile::getEdge(int side)
{
    if (numSides() > side)
        return epoly[side]->getLine();
    else
        return QLineF();
}

qreal Tile::edgeLen(int side)
{
    if (numSides() > side)
        return epoly[side]->getLine().length();
    else
        return 0.0;
}

void Tile::legacyDecompose()
{
    epoly.clear();
    epoly = base.recreate();

    if (!Loose::zero(rotation))
        base.rotate(-rotation);
    if (!Loose::equals(scale,1.0))
        base.scale(1.0/scale);
}


