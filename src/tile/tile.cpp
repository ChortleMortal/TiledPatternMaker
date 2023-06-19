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

#include "tile/tile.h"
#include "geometry/edge.h"
#include "geometry/vertex.h"
#include "geometry/loose.h"

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

    createEpolyFromBase();
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
    createEpolyFromBase();
}

void Tile::createRegularBase()
{
    base.clear();

    int idx     = 0;
    qreal angle = (M_PI / static_cast<qreal>(n)) * (static_cast<qreal>(2.0 * idx) + 1.0);
    qreal sc    = 1.0 / qCos( M_PI / static_cast<qreal>(n) );

    VertexPtr v  = make_shared<Vertex>(QPointF(sc * qCos(angle), sc * qSin(angle)));
    VertexPtr v1 = v;
    for( int idx = 1; idx < n; ++idx )
    {
        qreal angle2 = (M_PI / static_cast<qreal>(n)) * (static_cast<qreal>(2.0 * idx) + 1);
        VertexPtr v2 = make_shared<Vertex>(QPointF(sc * qCos(angle2), sc * qSin(angle2)));
        base.push_back(make_shared<Edge>(v1,v2));
        v1 = v2;
    }
    base.push_back(make_shared<Edge>(v1,v));
}

void Tile::createEpolyFromBase()
{
    epoly.clear();
    epoly = base.recreate();
    //qDebug()<< "Tile create 1" << epoly.getPoints();

    if (!Loose::zero(rotation))
    {
        epoly.rotate(rotation);
        //qDebug()<< "Tile create 2" << epoly.getPoints();
    }
    if (!Loose::equals(scale,1.0))
    {
        epoly.scale(scale);
        //qDebug()<< "Tile create 3" << epoly.getPoints();
    }
}

void Tile::decompose()
{
    // the base is really the created
    epoly.clear();
    epoly = base.recreate();

    if (!Loose::zero(rotation))
    {
        base.rotate(-rotation);
    }
    if (!Loose::equals(scale,1.0))
    {
        base.scale(1.0/scale);
    }
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

Tile::Tile(const Tile & other )
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

void Tile::setN(int n)
{
    this->n = n;
    if (regular)
    {
        createRegularBase();
        createEpolyFromBase();
    }
}

void Tile::setRotation(qreal rotate)
{
    rotation = rotate;
    createEpolyFromBase();
}

qreal Tile::getRotation()
{
    return rotation;
}

void Tile::deltaRotation(qreal delta)
{
    setRotation(rotation + delta);
}

void Tile::setScale(qreal scale)
{
    this->scale = scale;
    createEpolyFromBase();
}

qreal Tile::getScale()
{
    return scale;
}

void Tile::deltaScale(qreal delta)
{
    setScale(scale + delta);
}

void Tile::setRegular(bool enb)
{
    if (enb == regular)
        return;

    if (enb)
    {
        regular =  true;
        createRegularBase();
        createEpolyFromBase();
    }
    else
    {
        regular = false;
        createEpolyFromBase();
    }
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
}

TilePtr Tile::recreate()
{
    TilePtr f;
    if (regular)
    {
        f = make_shared<Tile>(n,rotation,scale);
    }
    else
    {
        f = make_shared<Tile>(base,rotation,scale);
    }
    return  f;
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
    for (auto & edge : qAsConst(epoly))
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
    for (auto & edge : qAsConst(base))
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

TilePtr Tile::copy()
{
    TilePtr fp = make_shared<Tile>(*this);
    return fp;
}
