#pragma once
#ifndef TILING_DATA
#define TILING_DATA
#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "settings/model_settings.h"

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;

typedef QVector<QTransform> Placements;
typedef QVector<PlacedTilePtr> PlacedTiles;

class TilingData
{
public:
    TilingData();
    ~TilingData();

    TilingData          copy();
    bool                isEmpty();

    const PlacedTiles & getInTiling() const   { return _tilingTiles; }
    PlacedTiles &       getRWInTiling()       { return _tilingTiles; }

    void                setTranslationVectors(QPointF t1, QPointF t2) { setTrans1(t1); setTrans2(t2); }
    void                setTrans1(QPointF pt) { t1 = pt; }
    void                setTrans2(QPointF pt) { t2 = pt; }
    QPointF             getTrans1() const { return t1; }
    QPointF             getTrans2() const { return t2; }

    const ModelSettings& getSettings() const { return settings; }
    ModelSettings     & getSettingsAccess() { return settings ;}

    const FillData &    getFillData() const { return settings.getFillDataAccess(); }
    FillData &          getFillDataAccess() { return settings.getFillData(); }

    Placements          getFillPlacemenets();

    QString             dump() const;

private:
    QPointF             t1;
    QPointF             t2;
    ModelSettings       settings;
    PlacedTiles         _tilingTiles;
};

#endif

