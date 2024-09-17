#pragma once
#ifndef TILING_DATA
#define TILING_DATA

#include <QtGlobal>
#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "model/settings/canvas_settings.h"
#include "sys/geometry/fill_region.h"

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::weak_ptr<class PlacedTile>     wPlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;

class TilingPlacements :  public QVector<PlacedTilePtr>
{
public:
    void dump() const;
};

class TilingUnit : public  QVector<QPair<TilePtr,TilingPlacements>>
{
public:
    TilingUnit(){}
    TilingUnit(TilingPlacements & ptiles) { group(ptiles); }

    bool operator ==(const TilingUnit &other) const;

    void        set(TilingUnit & other) { *this = other;}
    void        group(TilingPlacements & ptiles);
    void        add(PlacedTilePtr placedTile);
    void        remove(PlacedTilePtr placedTile);

    TilingUnit  copy();

    int         numIncluded() const;
    int         numExcluded() const;
    int         numAll() const;

protected:

private:
    bool               containsTile(TilePtr fp);
    TilingPlacements & getTilePlacements(TilePtr fp);
};

class TilingData
{
public:
    TilingData() {}
    TilingData(const TilingData & other);
    ~TilingData();

    TilingData & operator=(const TilingData & other);

    bool operator ==(const TilingData &other) const;

    void                    clear();
    TilingData              copy();
    bool                    isEmpty();

    TilingUnit &            getTilingUnit2() { return _tilingUnit; }
    TilingPlacements        getIncluded() const;
    TilingPlacements        getExcluded() const;
    TilingPlacements        getAll() const;

    void                    addPlacedTile(const PlacedTilePtr ptp);
    void                    removePlacedTile(PlacedTilePtr ptp);

    QPointF                 getTranslateOrigin() { return _translateOrigin; }
    void                    setTranslationVectors(QPointF t1, QPointF t2, QPointF origin);
    QPointF                 getTrans1() const { return _t1; }
    QPointF                 getTrans2() const { return _t2; }
    Placements              getFillPlacements();

    const CanvasSettings &  getCanvasSettings() const { return _settings; }
    void                    setCanvasSettings(const CanvasSettings & settings);


    QString                 info() const;

private:
    QPointF                 _t1;
    QPointF                 _t2;
    QPointF                 _translateOrigin;   // used by tiling maker view - not by mosaic
    CanvasSettings          _settings;
    TilingUnit              _tilingUnit;       // these are the included and excluded placed tiles
};
#endif

