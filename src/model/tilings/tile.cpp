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

Tile::Tile(EdgePoly ep) : EdgePoly(ep)
{
    // createas irregular tile
    n          = ep.epoly.size();
    regular    = false;
    refs++;
}

Tile::Tile(int n, qreal rotate, qreal scale) : EdgePoly()
{
    // Create an n-sided regular polygon with a vertex at (1,0).
    this->n     = n;
    regular     = true;
    refs++;

    createRegularBase();
    rotation    = rotate;
    this->scale = scale;
    compose();
}

Tile::Tile(const TilePtr other ) : EdgePoly(*other.get())
{
    n           = other->n;
    regular     = other->regular;
    refs++;
}

Tile::Tile(const Tile & other ) : EdgePoly(other)
{
    n       = other.n;
    regular = other.regular;
    refs++;
}

Tile::~Tile()
{
    refs--;
}

bool Tile::operator == (const Tile & other) const
{
    if (n != other.n)
        return false;

    if (regular  != other.regular)
        return false;

    const EdgePoly & myp  = getEdgePoly();
    const EdgePoly & othp = other.getEdgePoly();

    if (myp != othp)
        return false;

    return true;
}

bool Tile::isSimilar(const TilePtr other)
{
    if (regular != other->regular)
        return false;
    if (numEdges() != other->numEdges())
        return false;
    if (!Loose::equals(rotation, other->rotation))
        return false;
    if (!Loose::equals(scale, other->scale))
        return false;
    return true;
}

// creates new base and composes
TilePtr Tile::recreate()
{
    TilePtr tp;
    if (regular)
    {
        tp = make_shared<Tile>(n,rotation,scale);
    }
    else
    {
        tp = make_shared<Tile>(EdgePoly::recreate());
    }
    return  tp;
}

// stright copy of base and does not compose
TilePtr Tile::copy()
{
    TilePtr tp;
    if ( regular)
    {
        tp = make_shared<Tile>(n,rotation,scale);
    }
    else
    {
        EdgePoly ep(*this);;
        tp = make_shared<Tile>(ep);
    }
    return tp;
}

void Tile::setN(uint n)
{
    if (n==0 || n==2)
        return;

    this->n = n;
    if (regular)
    {
        createRegularBase();
        compose();
    }
}

void Tile::deltaScale(qreal delta)
{
    setScale(scale + delta);
}

void Tile::deltaRotation(qreal delta)
{
    setRotate(rotation + delta);
}

void Tile::setRegular(bool enb)
{
    regular = enb;
}

void Tile::flipRegularity()
{
    if (!regular)
    {
        regular =  true;
        createRegularBase();
    }
    else
    {
        regular = false;
    }
}

void Tile::createRegularBase()
{
    QPolygonF poly = Geo::getCircumscribedPolygon(n);
    EdgePoly::set(poly);
}

QString Tile::toString() const
{
    QString text;
    QTextStream str(&text);
    int i=0;

    str << "{ ";
    for (auto & edge : epoly)
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
    for (auto & edge : base)
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
    return QString("%1%2 ").arg(numEdges()).arg((regular) ? 'r' : 'i' );
}

QPointF Tile::getCenter()
{
    if (regular)
        return calcCenter();
    else
        return calcIrregularCenter();
}

QLineF Tile::getEdge(uint side)
{
    if (numEdges() > side)
        return epoly.at(side)->getLine();
    else
        return QLineF();
}

qreal Tile::edgeLen(uint side)
{
    if (numEdges() > side)
        return epoly.at(side)->getLine().length();
    else
        return 0.0;
}



