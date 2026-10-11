#include <QDebug>

#include "model/motifs/extender.h"
#include "model/motifs/motif.h"
#include "sys/geometry/map.h"
#include "sys/geometry/neighbour_map.h"
#include "sys/geometry/neighbours.h"
#include "model/tilings/tile.h"

///////////////////////////////////////////////////////////////////////////////
///
///     Extender
///
///////////////////////////////////////////////////////////////////////////////

Extender::Extender(Motif *motif) : _extendedBoundary(motif->getN())
{
    this->motif         = motif;
    _extendRays         = false;
    _extendTipsToBound  = false;
    _extendTipsToTile   = false;
    _connectRays        = 0;
    _embedBoundary      = false;
    _embedTile          = false;
    _stellation         = 0;
    _fitToTile          = false;
}

Extender::Extender(const Extender & other) : _extendedBoundary(other._extendedBoundary)
{
    this->motif         = other.motif;
    _extendRays         = other._extendRays;
    _extendTipsToBound  = other._extendTipsToBound;
    _extendTipsToTile   = other._extendTipsToTile;
    _connectRays        = other._connectRays;
    _embedBoundary      = other._embedBoundary;
    _embedTile          = other._embedTile;
    _stellation         = other._stellation;
    _fitToTile          = other._fitToTile;
}

Extender& Extender::operator=(const Extender& rhs)
{
    motif               = rhs.motif;
    _extendRays         = rhs._extendRays;
    _extendTipsToBound  = rhs._extendTipsToBound;
    _extendTipsToTile   = rhs._extendTipsToTile;
    _connectRays        = rhs._connectRays;
    _embedBoundary      = rhs._embedBoundary;
    _embedTile          = rhs._embedTile;
    _stellation         = rhs._stellation;
    _fitToTile          = rhs._fitToTile;

    _extendedBoundary   = rhs._extendedBoundary;
    return *this; }


bool Extender::equals(ExtenderPtr other)
{
    if (!_extendedBoundary.equals(other->_extendedBoundary))
        return false;

    if (_extendRays != other->_extendRays)
        return false;

    if (_extendTipsToBound != other->_extendTipsToBound)
        return false;

    if (_extendTipsToTile  != other->_extendTipsToTile)
        return false;

    if (_connectRays != other->_connectRays)
        return false;

    if (_embedBoundary != other->_embedBoundary)
        return false;

    if (_embedTile != other->_embedTile)
        return false;

    if (_stellation != other->_stellation)
        return false;

    if (_fitToTile != other->_fitToTile)
        return false;
    return true;
}

bool Extender::isUnity()
{
    // unity extnders have no value and can be deleted on load/save

    if (_extendRays)        return false;
    if (_extendTipsToBound) return false;
    if (_extendTipsToTile)  return false;
    if (_connectRays)       return false;
    if (_stellation)        return false;
    if (_embedBoundary)     return false;
    if (_embedTile)         return false;
    if (_fitToTile)         return false;

    return _extendedBoundary.isUnity(motif);
}

void Extender::buildExtendedBoundary()
{
    if (motif->isRadial())
    {
        _extendedBoundary.buildRadial(motif->getTile());
    }
    else
    {
        _extendedBoundary.buildExplicit(motif->getTile());
    }
}

void Extender::embedBoundaryInMap(MapPtr motifMap)
{
    Q_ASSERT(_embedBoundary);

    buildExtendedBoundary();

    const ExtendedBoundary & eb = getExtendedBoundary();
    MapPtr ebmap = std::make_shared<Map>("Temp",eb.getPoly());
    motifMap->mergeMap(ebmap);
}

void Extender::embedTileInMap(MapPtr motifMap,TilePtr tile)
{
    Q_ASSERT(_embedTile);

    for (QLineF & line : tile->getLines())
    {
        VertexPtr v1 = motifMap->insertVertex(line.p1());
        VertexPtr v2 = motifMap->insertVertex(line.p2());
        motifMap->insertEdge(v1,v2);
    }
}

qreal Extender::len(VertexPtr v1, VertexPtr v2)
{
    return QLineF(v1->pt,v2->pt).length();
}

// From Chat GPT
QLineF Extender::perpLine(QLineF line, QPointF pt)
{
    // Define the points of the original line
    QPointF point1 = line.p1();
    QPointF point2 = line.p2();

    // Calculate the slope of the original line
    qreal dx = point2.x() - point1.x();
    qreal dy = point2.y() - point1.y();
    qreal originalSlope = dy / dx;

    // Calculate the perpendicular slope
    qreal perpendicularSlope = -1 / originalSlope;

    // Define the length of the perpendicular line segment
    qreal length = 100;

    // Calculate the endpoints of the perpendicular line
    qreal theta = std::atan(perpendicularSlope);
    QPointF perpPoint1(pt.x() + length * std::cos(theta), pt.y() + length * std::sin(theta));
    QPointF perpPoint2(pt.x() - length * std::cos(theta), pt.y() - length * std::sin(theta));

    // Draw the perpendicular line
    return QLineF(perpPoint1, perpPoint2);
}

void Extender::dump()
{
    qInfo().noquote()
    << "Extender:"
    << "Rays:"                << _extendRays
    << "TipsToBoundary:"      << _extendTipsToBound
    << "TipsToTile:"          << _extendTipsToTile
    << "ConnectRaya:"         << _connectRays
    << "EmbedBoundary:"       << _embedBoundary
    << "EmbedTile:"           << _embedTile
    << "Stellation:"          << _stellation
    << "FitToTile:"           << _fitToTile;
}


