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

#include <QString>
#include <QTransform>
#include <QTextStream>
#include "misc/pugixml.hpp"
#include "geometry/edgepoly.h"

typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;
typedef std::shared_ptr<class Tile>         TilePtr;

class Tiling;

class PlacedTile
{
public:
    // Creation.
    PlacedTile();
    PlacedTile(TilePtr tile, QTransform T);
    ~PlacedTile() {}

    PlacedTilePtr copy();

    // Data.
    void             setTransform(QTransform newT);
    void             setTile(TilePtr tile);
    TilePtr          getTile();
    QTransform       getTransform();
    EdgePoly         getTileEdgePoly();
    QPolygonF        getTilePoints();
    EdgePoly         getPlacedEdgePoly();
    QPolygonF        getPlacedPoints();

    bool saveAsGirihShape(QString name);
    bool loadFromGirihShape(QString name);

    bool isGirihShape() { return !girihShapeName.isEmpty(); }
    QString getGirihShapeName() { return girihShapeName; }

    bool show() { return _show; }
    void setShow(bool show) { _show = show; }

protected:
    void saveGirihShape(QTextStream & out, QString name);
    void loadGirihShape(pugi::xml_node & poly_node);
    void loadGirihShape(int sides, pugi::xml_node & poly_node);
    void loadGirihShapeOld(pugi::xml_node & poly_node);

private:
    QString     girihShapeName;
    TilePtr     tile;
    QTransform  T;
    bool        _show;  // used by tiling maker view
};
#endif
