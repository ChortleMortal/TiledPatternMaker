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

#include "model/motifs/motif.h"
#include "model/tilings/tile.h"
#include "sys/geometry/map.h"
#include "sys/geometry/map_cleanser.h"

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
    motifType     = Type::Undefined;
    motifScale    = 1.0;
    motifRotate   = 0.0;
    _n            = 10;
    _motifVersion = 1;          // default
    _motifDebug   = 0;
    cleanseVal    = 0;
    sensitivity   = 1e-2;
}

Motif::Motif(const Motif & other)
{
    refs++;

    motifType         = other.motifType;
    motifRotate       = other.motifRotate;
    motifScale        = other.motifScale;
    cleanseVal        = other.cleanseVal;
    sensitivity       = other.sensitivity;

    _tile             = other._tile;
    _n                = other._n;
    _motifBoundary    = other._motifBoundary;
    _motifVersion     = other._motifVersion;
    _motifDebug       = other._motifDebug;
    // extenders are not handled here
}

Motif::~Motif()
{
    //qDebug() << "Figure destructor" << this;
    refs--;
}

void Motif::resetMotifMap()
{
    motifMap.reset();
}

bool Motif::hasValidMotifMap()
{
    return (motifMap && !motifMap->isEmpty());
}

// Get a complete map from unit.
MapPtr Motif::getMotifMap()
{
    //qDebug() << "Motif::getMap";
    if (!motifMap || motifMap->isEmpty())
    {
        build();
    }
    return motifMap;
}

MapPtr Motif::getValidMotifMap()
{
    return (hasValidMotifMap()) ? motifMap : nullptr;
}

MapPtr Motif::getExistingMotifMap()
{
    return motifMap;
}

// called at the end of build() by all motif types
void Motif::cleanseMotifMap()
{
    if (cleanseVal > 0 && motifMap && !motifMap->isEmpty())
    {
        MapCleanser mc(motifMap);
        mc.cleanse(cleanseVal,sensitivity);
    }
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

    uint index = 0;
    for (auto & extender : _extenders)
    {
        if (!extender->equals(other->getExtender(index++)))
            return false;
    }

    return true;
}

bool Motif::isIrregular() const
{
    switch (motifType)
    {
    case Type::MapMotif:
    case Type::Inferred:
    case Type::AdaptiveRosette:
    case Type::Hourglass:
    case Type::Intersect:
    case Type::Girih:
    case Type::AdaptiveStar:
    case Type::ExplicitTile:
    case Type::NoMapMotif:
        return true;
    default:
        return false;
    }
}

bool Motif::isRadial() const
{
    switch (motifType)
    {
    case Type::Radial:
    case Type::Rosette:
    case Type::Rosette2:
    case Type::Star:
    case Type::Star2:
        return true;
    default:
        return false;
    }
}

void Motif::addExtender(ExtenderPtr ep)
{
    _extenders.push_back(ep);
}

void Motif::deleteExtender(ExtenderPtr extender)
{
    _extenders.removeAll(extender);
}

ExtenderPtr Motif::getExtender(uint i)
{
    if (i < (uint)_extenders.size())
        return _extenders[i];
    else
    {
        ExtenderPtr ep;
        return ep;
    }
}

int Motif::modulo(int i, int sz)
{
    if (i >= 0)
        return i % sz;
    else
        return (sz >= 0 ? sz : -sz) - 1 + (i + 1) % sz;
}

const QString sMotifType[] =
    {
        "Undefined Motif",
        "Radial Motif",
        "Rosette Motif",
        "Rosette2 Motif",

        "Star Motif",
        "Star2 Motif",
        "Jeweled Star Motif",

        "Explicit Map Motif",
        "Adaptive Inferred Motif",
        "Adaptive Rosette Motif",
        "Adaptive Hourglass Motif",

        "Adaptive Intersect Motif",
        "Adaptive Girih Motif",
        "Adaptive Star Motif",

        "Explicit Tile Motif",
        "No Motif (Empty)"
};

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
