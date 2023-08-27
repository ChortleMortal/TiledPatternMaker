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
#include "geometry/edge.h"
#include "geometry/map.h"
#include "geometry/transform.h"
#include "tile/tile.h"

using std::make_shared;

int Motif::refs = 0;

//////////////////////////////////////////////////////////////////
///
/// Motif
///
//////////////////////////////////////////////////////////////////

Motif::Motif()
{
    refs++;
    motifType     = MOTIF_TYPE_UNDEFINED;
    motifScale    = 1.0;
    motifRotate   = 0.0;
    _n            = 10;
    version       = 1;          // default
}

Motif::Motif(const Motif & other)
{
    refs++;

    motifType         = other.motifType;
    motifRotate       = other.motifRotate;
    motifScale        = other.motifScale;
    tile              = other.tile;
    _n                = other._n;
    _motifBoundary    = other._motifBoundary;
    _extendedBoundary = other._extendedBoundary;
    version           = other.version;
}

Motif::Motif(MotifPtr other)
{
    refs++;

    motifType         = other->motifType;
    motifRotate       = other->motifRotate;
    motifScale        = other->motifScale;
    tile              = other->tile;
    _n                = other->_n;
    _motifBoundary    = other->_motifBoundary;
    _extendedBoundary = other->_extendedBoundary;
    version           = other->version;
}

Motif::~Motif()
{
    //qDebug() << "Figure destructor" << this;
    refs--;
}

//////////////////////////////////////////////////////////////////
///
/// MotifData
///
//////////////////////////////////////////////////////////////////

bool Motif::equals(const  MotifPtr other)
{
    if (motifType      != other->motifType)
        return false;
    if (_n             != other->_n)
        return false;
    if (motifScale     != other->motifScale)
        return false;
    if (motifRotate    != other->motifRotate)
        return false;
    if (_motifBoundary  != other->_motifBoundary)
        return false;
    if (!_extendedBoundary.equals(other->_extendedBoundary))
        return false;
    return true;
}

bool Motif::isIrregular() const
{
    switch (motifType)
    {
    case MOTIF_TYPE_EXPLICIT_MAP:
    case MOTIF_TYPE_INFERRED:
    case MOTIF_TYPE_IRREGULAR_ROSETTE:
    case MOTIF_TYPE_HOURGLASS:
    case MOTIF_TYPE_INTERSECT:
    case MOTIF_TYPE_GIRIH:
    case MOTIF_TYPE_IRREGULAR_STAR:
    case MOTIF_TYPE_EXPLCIT_TILE:
    case MOTIF_TYPE_IRREGULAR_NO_MAP:
        return true;
    default:
        return false;
    }
}

bool Motif::isRadial() const
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

void Motif::buildExtendedBoundary()
{
    if (isRadial())
    {
        _extendedBoundary.buildRadial();
    }
    else
    {
        _extendedBoundary.buildExplicit(tile);
    }
}

void Motif::buildMotifBoundary(TilePtr tile)
{
    if (isRadial())
    {
        if (getN() >=3)
        {
            Tile f(getN(),getMotifRotate(), getMotifScale());
            QPolygonF p = f.getPolygon();
            setMotifBoundary(p);
        }
    }
    else
    {
        QPointF pt  = tile->getCenter();

        QTransform t;
        if (!Loose::equals(motifScale,1.0))
        {
            t *= Transform::scaleAroundPoint(pt,motifScale);
        }
        if (!Loose::zero(motifRotate))
        {
            t *= Transform::rotateDegreesAroundPoint(pt,motifRotate);
        }

        QPolygonF poly  = tile->getPoints();
        if (!t.isIdentity())
        {
            poly = t.map(poly);
        }

        setMotifBoundary(poly);
    }
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
        if (debugMap)
            debugMap->insertDebugMark(p, QString::number(i++));
    }
}

int Motif::modulo(int i, int sz)
{
    if (i >= 0)
        return i % sz;
    else
        return (sz >= 0 ? sz : -sz) - 1 + (i + 1) % sz;
}



///////////////////////////////////////////////////////////
///
/// Points
///
///////////////////////////////////////////////////////////

Points::Points(const QPolygonF & other)
{
    for (auto i = 0; i < other.size(); i++)
    {
        *this << other[i];
    }
}

void Points::translate(QPointF p)
{
    for (auto it = begin(); it != end(); it++)
    {
        *it += p;
    }
}

void Points::transform(QTransform t)
{
    for (auto it = begin(); it != end(); it++)
    {
        auto pt = *it;
        pt = t.map(pt);
        *it = pt;
    }
}
