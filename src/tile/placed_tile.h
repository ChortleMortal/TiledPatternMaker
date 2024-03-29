#pragma once
#ifndef PLACED_TILE
#define PLACED_TILE

////////////////////////////////////////////////////////////////////////////
//
// placedTile.java
//
// A placedTile is a Tile together with a transform matrix.
// It allows us to share an underlying feature while putting several
// copies together into a tiling.  A tiling is represented as a
// collection of placedTiles (that may share Tiles) that together
// make up a translational unit.

#include <QObject>
#include <QString>
#include <QTransform>
#include <QTextStream>
#include "misc/pugixml.hpp"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;
typedef std::shared_ptr<class Tile>         TilePtr;

class Tiling;

class PlacedTile : public QObject
{
    enum eTileState
    {
        UNDEFINED   = 0x00,
        TOUCHING    = 0x01,
        OVERLAPPING = 0x02,
    };

    Q_OBJECT

public:

    // Creation.
    PlacedTile();
    PlacedTile(TilePtr tile, QTransform T);
    ~PlacedTile() {}

    PlacedTilePtr copy();

    // Data.
    void            setTransform(QTransform newT);
    void            setTile(TilePtr tile);
    void            setShow(bool show);
    bool            loadFromGirihShape(QString name);

    TilePtr         getTile();
    QTransform      getTransform();
    EdgePoly        getTileEdgePoly();
    QPolygonF       getTilePoints();
    EdgePoly        getPlacedEdgePoly();
    QPolygonF       getPlacedPoints();

    bool            saveAsGirihShape(QString name);
    bool            isGirihShape()      { return !girihShapeName.isEmpty(); }
    QString         getGirihShapeName() { return girihShapeName; }

    bool show()             { return _show; }

    void clearViewState()   { _viewState  = UNDEFINED; }
    void setOverlapping()   { _viewState |= OVERLAPPING; }
    void setTouching()      { _viewState |= TOUCHING; }
    bool isOverlapping()    { return (_viewState & OVERLAPPING); }
    bool isTouching()       { return (_viewState & TOUCHING); }

signals:
    void sig_tileChanged();

protected:
    void saveGirihShape(QTextStream & out, QString name);
    void loadGirihShape(pugi::xml_node & poly_node);
    void loadGirihShape(int sides, pugi::xml_node & poly_node);
    void loadGirihShapeOld(pugi::xml_node & poly_node);

private:
    QString     girihShapeName;
    TilePtr     tile;
    QTransform  T;
    bool        _show;  // used by tiling maker
    uint        _viewState;
};
#endif
