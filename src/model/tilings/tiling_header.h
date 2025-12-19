#pragma once
#ifndef TILING_HEADER
#define TILING_HEADER

#include <QtGlobal>
#include <QObject>

#include "model/settings/canvas_settings.h"
#include "sys/geometry/fill_region.h"

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::weak_ptr<class PlacedTile>     wPlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;

class TilingHeader
{
public:
    TilingHeader() {}
    TilingHeader(const TilingHeader & other);
    ~TilingHeader();

    TilingHeader & operator=(const TilingHeader & other);

    bool operator ==(const TilingHeader &other) const;
    bool operator != (const TilingHeader & other) const { return !(*this == other); }

    bool isEmpty();

    TilingHeader            uniqueCopy();

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
    CanvasSettings          _settings;

    QPointF                 _translateOrigin;   // used by tiling maker view - not by mosaic
};
#endif

