#include <QtWidgets>
#include "model/tilings/tiling.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling_data.h"
#include "model/tilings/placed_tile.h"

using std::make_shared;

TilingData::TilingData(const TilingData & other)
{
    _t1             = other._t1;
    _t2             = other._t2;
    _settings       = other._settings;
    _tilingUnit     = other._tilingUnit;
}

TilingData::~TilingData()
{
    _tilingUnit.clear();
}

TilingData & TilingData::operator=(const TilingData & other)
{
    _t1             = other._t1;
    _t2             = other._t2;
    _settings       = other._settings;
    _tilingUnit     = other._tilingUnit;
    return *this;
}

bool TilingData::operator==(const TilingData & other) const
{
    if (_t1 != other._t1)
        return false;
    if (_t2 != other._t2)
        return false;
    if ( _settings != other._settings)
        return false;
    if (_tilingUnit.size() != other._tilingUnit.size())
        return false;
    if (_tilingUnit != other._tilingUnit)
        return false;
    return true;
}

TilingData TilingData::copy()
{
    TilingData td;
    td._settings        = _settings;
    td._t1              = _t1;
    td._t2              = _t2;
    td._translateOrigin = _translateOrigin;
    td._tilingUnit      = _tilingUnit.copy();
    return td;
}

TilingPlacements TilingData::getIncluded() const
{
    TilingPlacements placements;
    for (auto & apair : _tilingUnit)
    {
        const TilingPlacements & tps = apair.second;

        for (auto & placed : tps)
        {
            if (placed->isIncluded())
            {
                placements.push_back(placed);
            }
        }
    }
    return placements;
}

TilingPlacements TilingData::getExcluded() const
{
    TilingPlacements placements;
    for (auto & apair : _tilingUnit)
    {
        const TilingPlacements & tps = apair.second;

        for (auto & placed : tps)
        {
            if (!placed->isIncluded())
            {
                placements.push_back(placed);
            }
        }
    }
    return placements;
}

TilingPlacements  TilingData::getAll() const
{
    TilingPlacements placements;
    for (auto & apair : _tilingUnit)
    {
        const TilingPlacements & tps = apair.second;

        for (auto & placed : tps)
        {
            placements.push_back(placed);
        }
    }
    return placements;
}

void TilingData::setTranslationVectors(QPointF t1, QPointF t2, QPointF origin)
{
    _t1 = t1;
    _t2 = t2;
    _translateOrigin = origin;
    Sys::setTilingChange();
}

void TilingData::addPlacedTile(const PlacedTilePtr ptp)
{
    _tilingUnit.add(ptp);
    Sys::setTilingChange();
}

void TilingData::removePlacedTile(PlacedTilePtr ptp)
{
    _tilingUnit.remove(ptp);
    Sys::setTilingChange();
}

void TilingData::clear()
{
    _tilingUnit.clear();
    Sys::setTilingChange();
}

void  TilingData::setCanvasSettings(const CanvasSettings & settings)
{
    _settings = settings;
    Sys::setTilingChange();
}


bool TilingData::isEmpty()
{
    if (_tilingUnit.isEmpty() && _t1.isNull() && _t2.isNull())
        return true;
    else
        return false;
}

Placements TilingData::getFillPlacements()
{
    Placements placements;
    const FillData & fd = _settings.getFillData();
    int minX,minY,maxX,maxY;
    bool singleton;
    fd.get(singleton,minX,maxX,minY,maxY);
    if (!singleton)
    {
        for (int h = minX; h <= maxX; h++)
        {
            for (int v = minY; v <= maxY; v++)
            {
                QPointF pt   = (_t1 * static_cast<qreal>(h)) + (_t2 * static_cast<qreal>(v));
                QTransform T = QTransform::fromTranslate(pt.x(),pt.y());
                placements << T;
            }
        }
    }
    else
    {
        placements << QTransform();
    }
    return placements;
}

QString TilingData::info() const
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "t1=" << _t1 << "t2=" << _t2 << "num tiles=" << _tilingUnit.numAll();
    return astring;
}

///////////////////////////////////////////////////////////////////////////
///
/// Tiling Unit
///
///////////////////////////////////////////////////////////////////////////

void TilingPlacements::dump() const
{
    for (const PlacedTilePtr & ptp : std::as_const(*this))
    {
        ptp->dump();
    }
}

///////////////////////////////////////////////////////////////////////////
///
/// TileGroup
///
///////////////////////////////////////////////////////////////////////////

void TilingUnit::group(TilingPlacements & ptiles)
{
    for (const PlacedTilePtr & placedTile : std::as_const(ptiles))
    {
        add(placedTile);
    }
}

bool TilingUnit::operator ==(const TilingUnit &other) const
{
    int numthis = count();
    if (numthis != other.count())
        return false;
    for (int i=0; i < numthis; i++)
    {
        const QPair<TilePtr,TilingPlacements> & thisPair  = this->at(i);
        const QPair<TilePtr,TilingPlacements> & otherPair = other.at(i);
        if (*thisPair.first.get() != *otherPair.first.get())
            return false;
        const TilingPlacements &  myp = thisPair.second;
        const TilingPlacements & othp = otherPair.second;
        int numPl = myp.count();
        if (numPl != othp.count())
            return false;
        for (int j=0; j < numPl; j++)
        {
            if (*myp[j].get() != *othp[j].get())
                return false;
        }
    }
    return true;
}

void TilingUnit::add(PlacedTilePtr placedTile)
{
    TilePtr tile = placedTile->getTile();
    if (containsTile(tile))
    {
        TilingPlacements & v = getTilePlacements(tile);
        v.push_back(placedTile);
    }
    else
    {
        TilingPlacements tps;
        tps.push_back(placedTile);
        QPair<TilePtr,TilingPlacements> apair(tile,tps);
        push_back(apair);
    }
}

void TilingUnit::remove(PlacedTilePtr placedTile)
{
    TilePtr tile = placedTile->getTile();
    for (auto & apair : *this)
    {
        if (apair.first == tile)
        {
            TilingPlacements & tps = apair.second;
            if (tps.contains(placedTile))
            {
                tps.removeOne(placedTile);
            }
            return;
        }
    }
}

TilingUnit  TilingUnit::copy()
{
    TilingUnit acopy;
    for (auto & apair : *this)
    {
        TilePtr newTile = make_shared<Tile>(apair.first);
        TilingPlacements newPs;
        TilingPlacements & tps = apair.second;
        for (auto  & placed : tps)
        {
            PlacedTilePtr ptp = placed->copy();
            ptp->setTile(newTile);
            newPs.push_back(ptp);
        }
        QPair<TilePtr,TilingPlacements> newpair(newTile,newPs);
        acopy.push_back(newpair);
    }

    return acopy;
}

bool TilingUnit::containsTile(TilePtr fp)
{
    for (const auto & apair : std::as_const(*this))
    {
        TilePtr f = apair.first;
        if (f == fp)
        {
            return true;
        }
    }
    return false;
}

TilingPlacements & TilingUnit::getTilePlacements(TilePtr fp)
{
    Q_ASSERT(containsTile(fp));

    for (auto & apair : *this)
    {
        TilePtr f = apair.first;
        if (f == fp)
        {
            return apair.second;
        }
    }

    qWarning("should never reach here");
    static TilingPlacements tpls;
    return tpls;
}


int TilingUnit::numIncluded() const
{
    int num = 0;
    for (auto & apair : *this)
    {
        const TilingPlacements & tps = apair.second;
        for (auto & placed : tps)
        {
            if (placed->isIncluded())
            {
                num++;
            }
        }
    }
    return num;
}

int TilingUnit::numExcluded() const
{
    int num = 0;
    for (auto & apair : *this)
    {
        const TilingPlacements & tps = apair.second;
        for (auto & placed : tps)
        {
            if (!placed->isIncluded())
            {
                num++;
            }
        }
    }
    return num;
}

int TilingUnit::numAll() const
{
    int num = 0;
    for (auto & apair : *this)
    {
        const TilingPlacements & tps = apair.second;
        num += tps.count();
    }
    return num;
}



