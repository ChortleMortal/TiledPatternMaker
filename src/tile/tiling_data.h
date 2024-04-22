#pragma once
#ifndef TILING_DATA
#define TILING_DATA

#include <QtGlobal>
#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "settings/canvas_settings.h"
#include "geometry/fill_region.h"

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::weak_ptr<class PlacedTile>     wPlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;

typedef QVector<PlacedTilePtr> PlacedTiles;

class TilingData : public QObject
{
    Q_OBJECT

public:
    TilingData();
    TilingData(const TilingData & other);
    ~TilingData();

     TilingData & operator=(const TilingData & other);

    void                    setTranslationVectors(QPointF t1, QPointF t2, QPointF origin);
    QPointF                 getTranslateOrigin() { return _translateOrigin; }
    void                    setCanvasSettings(const CanvasSettings & settings);

    void                    add(const PlacedTilePtr ptp);
    void                    add(PlacedTiles & tiles);
    void                    remove(const PlacedTilePtr ptp);
    void                    clear();

    TilingData              copy();
    bool                    isEmpty();

    const PlacedTiles &     getInTiling() const { return _placedTiles; }
    const CanvasSettings &  getSettings() const { return _settings; }
    Placements              getFillPlacements();

    QPointF                 getTrans1() const { return _t1; }
    QPointF                 getTrans2() const { return _t2; }

    QString                 info() const;

signals:
    void                    sig_tilingChanged();

private:
    QPointF                 _t1;
    QPointF                 _t2;
    QPointF                 _translateOrigin;   // used by tiling maker view - not by mosaic
    CanvasSettings          _settings;
    PlacedTiles             _placedTiles;   // these are in-tiling aka included
};

#endif

