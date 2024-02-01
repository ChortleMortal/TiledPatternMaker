#include <QtWidgets>
#include "tile/tiling.h"
#include "tile/tiling_data.h"
#include "tile/placed_tile.h"

using std::make_shared;

TilingData::TilingData()
{
}

TilingData::~TilingData()
{
    _placedTiles.clear();
}

TilingData::TilingData(const TilingData & other)
{
    _t1             = other._t1;
    _t2             = other._t1;
    _settings       = other._settings;
    _placedTiles    = other._placedTiles;
}

TilingData & TilingData::operator=(const TilingData & other)
{
    _t1             = other._t1;
    _t2             = other._t1;
    _settings       = other._settings;
    _placedTiles    = other._placedTiles;
    return *this;
}

void TilingData::setTranslationVectors(QPointF t1, QPointF t2)
{
    _t1 = t1;
    _t2 = t2;
    emit sig_tilingChanged();
}

void TilingData::add(const PlacedTilePtr ptp)
{
    _placedTiles.push_back(ptp);
    emit sig_tilingChanged();
}

void TilingData::add(PlacedTiles & tiles)
{
    _placedTiles += tiles;
    emit sig_tilingChanged();
}

void TilingData::remove(const PlacedTilePtr ptp)
{
    _placedTiles.removeOne(ptp);
    emit sig_tilingChanged();
}

void TilingData::clear()
{
    _placedTiles.clear();
    emit sig_tilingChanged();
}

void  TilingData::setCanvasSettings(const CanvasSettings & settings)
{
    _settings = settings;
    emit sig_tilingChanged();
}

TilingData TilingData::copy()
{
    TilingData td;
    td._settings = _settings;
    td._t1       = _t1;
    td._t2       = _t2;

    for (const auto & placed : std::as_const(_placedTiles))
    {
        auto placed2 = placed->copy();
        td._placedTiles.push_back(placed2);
    }
    return td;
}

bool TilingData::isEmpty()
{
    if (getInTiling().isEmpty() && _t1.isNull() && _t2.isNull())
        return true;
    else
        return false;
}

const Placements TilingData::getFillPlacements()
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


QString TilingData::dump() const
{
    QString astring;
    QDebug  deb(&astring);

    deb << "t1=" << _t1 << "t2=" << _t2 << "num tiles=" << getInTiling().size();
    return astring;
}

