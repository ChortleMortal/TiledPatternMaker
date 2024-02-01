#pragma once
#ifndef TILING_DATA
#define TILING_DATA

#include <QtGlobal>
#include <QObject>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

#include "settings/canvas_settings.h"

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;

typedef QVector<QTransform> Placements;
typedef QVector<PlacedTilePtr> PlacedTiles;

class TilingData : public QObject
{
    Q_OBJECT

public:
    TilingData();
    TilingData(const TilingData & other);
    ~TilingData();

     TilingData & operator=(const TilingData & other);

    void                    setTranslationVectors(QPointF t1, QPointF t2);
    void                    setCanvasSettings(const CanvasSettings & settings);

    void                    add(const PlacedTilePtr ptp);
    void                    add(PlacedTiles & tiles);
    void                    remove(const PlacedTilePtr ptp);
    void                    clear();

    TilingData              copy();
    bool                    isEmpty();

    const PlacedTiles &     getInTiling() const { return _placedTiles; }
    const CanvasSettings &  getSettings() const { return _settings; }
    const Placements        getFillPlacements();

    QPointF                 getTrans1() const { return _t1; }
    QPointF                 getTrans2() const { return _t2; }

    QString                 dump() const;

signals:
    void                    sig_tilingChanged();

private:
    QPointF                 _t1;
    QPointF                 _t2;
    CanvasSettings          _settings;
    PlacedTiles             _placedTiles;   // these are in-tiling aka included
};

#endif

