#pragma once

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

#include "model/settings/tristate.h"
#include "sys/geometry/edge_poly.h"
#include "sys/sys/pugixml.hpp"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class PlacedTile>   PlacedTilePtr;
typedef std::shared_ptr<class Tile>         TilePtr;

class Tiling;

class PlacedTileState
{
public:
    PlacedTileState() { initState();}

    void initState()  { show = true; overlapping = false; touching = false; unique = false;}
    QString  info();

    bool            show;  // used by tiling maker
    bool            overlapping;
    bool            touching;
    Tristate        excluded;
    bool            unique;
};

class PlacedTile
{
public:
    PlacedTile();
    PlacedTile(TilePtr tile, QTransform T);
    ~PlacedTile() {}

    bool operator == (const PlacedTile & other);
    bool operator != (const PlacedTile & other) { return !(*this == other); }

    PlacedTilePtr   copy();

    void            setTile(TilePtr tile);
    TilePtr         getTile();

    void            setPlacement(QTransform newT);
    QTransform      getPlacement();

    void            setShow(bool show);
    bool            loadFromGirihShape(VersionedName vname);

    QTransform      getWorldTransform(const QTransform & distortion = QTransform());
    EdgePoly        getWorldEdgePoly(const QTransform & distortion = QTransform());
    QPolygonF       getWorldPoints(const QTransform & distortion = QTransform());

    PlacedTileState & getState() { return state; }

    // the tiling unit just contains True and False
    // when fill/flooded the addtional are Undefined
    bool            isExcluded()    { return state.excluded.get() == Tristate::eTristate::True; }
    bool            isIncluded()    { return state.excluded.get() == Tristate::eTristate::False; }
    bool            isTouching()    { return state.touching; }
    bool            isOverlapping() { return state.overlapping; }
    bool            isUnique()      { return state.unique; }

    void            include()       { state.excluded.set(Tristate::eTristate::False); }
    void            exclude()       { state.excluded.set(Tristate::eTristate::True); }
    bool            show()          { return state.show; }

    bool            saveAsGirihShape(VersionedName vname);
    bool            isGirihShape()      { return !girihShapeName.isEmpty(); }
    VersionedName   getGirihShapeName() { return girihShapeName; }
    void            setGirihShapeName(VersionedName vname) { girihShapeName = vname; }

    void            dump();
    QString         info();

protected:
    void saveGirihShape(QTextStream & out, VersionedName vname);
    void loadGirihShape(pugi::xml_node & poly_node);
    void loadGirihShape(int sides, pugi::xml_node & poly_node);
    void loadGirihShapeOld(pugi::xml_node & poly_node);

private:
    TilePtr         tile;
    QTransform      placement;
    VersionedName   girihShapeName;
    PlacedTileState state;
};

class PlacedTileSet : public QVector<PlacedTilePtr>
{
public:
    QString info();
};