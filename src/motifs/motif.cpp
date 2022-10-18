////////////////////////////////////////////////////////////////////////////
//
// Figure.java
//
// Making the user interface operate directly on maps would be
// a hassle.  Maps are a very low level geometry and topological
// structure.  Not so good for interacting with users.  So I
// define a Figure class, which is a higher level structure --
// an object that knows how to build maps.  Subclasses of Motif
// understand different ways of bulding maps, but have the advantage
// of being parameterizable at a high level.

#include "motifs/motif.h"
#include "tile/tile.h"
#include "geometry/map.h"
#include "geometry/edge.h"
//#include "geometry/transform.h"

using std::make_shared;

int Motif::refs = 0;

Motif::Motif()
{
    refs++;
    motifType           = MOTIF_TYPE_UNDEFINED;
    motifScale       = 1.0;
    motifRotate      = 0.0;
    display           = true;

    //figureMap = make_shared<Map>("figureMap");
}

Motif::Motif(const Motif & other)
{
    refs++;

    motifType           = other.motifType;
    n                 = other.n;

    motifScale       = other.motifScale;
    motifRotate      = other.motifRotate;

    motifBoundary   = other.motifBoundary;
    extendedBoundary = other.extendedBoundary;

    if (other.motifMap)
    {
        motifMap    = other.motifMap->copy();
    }

    display           = true;
}

Motif::~Motif()
{
    //qDebug() << "Figure destructor" << this;
    refs--;
}

bool Motif::equals(const  MotifPtr other)
{
    if (motifType != other->motifType)
        return false;
    if (motifScale     != other->motifScale)
        return false;
    if (motifRotate    != other->motifRotate)
        return false;
    if (motifBoundary  != other->motifBoundary)
        return false;
    if (!extendedBoundary.equals(other->extendedBoundary))
        return false;
    return true;
}

bool Motif::isExplicit()
{
    switch (motifType)
    {
    case MOTIF_TYPE_EXPLICIT:
    case MOTIF_TYPE_EXPLICIT_INFER:
    case MOTIF_TYPE_EXPLICIT_ROSETTE:
    case MOTIF_TYPE_EXPLICIT_HOURGLASS:
    case MOTIF_TYPE_EXPLICIT_INTERSECT:
    case MOTIF_TYPE_EXPLICIT_GIRIH:
    case MOTIF_TYPE_EXPLICIT_STAR:
    case MOTIF_TYPE_EXPLICIT_TILE:
        return true;
    default:
        return false;
    }
}

bool Motif::isRadial()
{
    switch (motifType)
    {
    case MOTIF_TYPE_RADIAL:
    case MOTIF_TYPE_ROSETTE:
    case MOTIF_TYPE_STAR:
    case MOTIF_TYPE_CONNECT_STAR:
    case MOTIF_TYPE_CONNECT_ROSETTE:
    case MOTIF_TYPE_EXTENDED_ROSETTE:
    case MOTIF_TYPE_EXTENDED_STAR:
        return true;
    default:
        return false;
    }
}

int Motif::getN()
{
    return n;
}

// uses existing tmpIndices
void Motif::annotateEdges()
{
    if (!motifMap)
    {
        return;
    }

    int i=0;
    for (auto & edge : qAsConst(motifMap->getEdges()))
    {
        QPointF p = edge->getMidPoint();
        debugMap->insertDebugMark(p, QString::number(i++));
    }
    //debugMap->dumpMap(false);
}

ExtendedBoundary::ExtendedBoundary()
{
    sides    = 1;   // defaults to a circle
    scale    = 1.0;
    rotate   = 0;
}

ExtendedBoundary::ExtendedBoundary(const ExtendedBoundary & other)
{
    sides    = other.sides;
    scale    = other.scale;
    rotate   = other.rotate;
    boundary2 = other.boundary2;
}

void ExtendedBoundary::set(const QPolygonF & p)
{
    Q_ASSERT(p.isClosed());
    boundary2 = p;
    //qDebug() << "set ext b" << boundary2;
}

const QPolygonF &ExtendedBoundary::get() const
{
    if (!boundary2.isEmpty())
    {
        Q_ASSERT(boundary2.isClosed());
    }
    //qDebug() << "get ext b" << boundary2;
    return boundary2;
}

bool ExtendedBoundary::equals(const ExtendedBoundary & other)
{
    return (sides == other.sides
            && scale == other.scale
            && rotate == other.rotate
            && boundary2 == other.boundary2);
}

void ExtendedBoundary::buildRadial()
{
    if (sides >= 3)
    {
        Tile f2(sides,rotate,scale);
        set(f2.getPolygon());
    }
    else
    {
        boundary2.clear();
    }
}

void ExtendedBoundary::buildExplicit(TilePtr tile)
{
    set(tile->getPolygon());
}
