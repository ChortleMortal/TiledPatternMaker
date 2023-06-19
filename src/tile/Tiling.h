#pragma once
#ifndef TILING
#define TILING

////////////////////////////////////////////////////////////////////////////
//
// Tiling.java
//
// The representation of a tiling, which will serve as the skeleton for
// Islamic designs.  A Tiling has two translation vectors and a set of
// placedTiles that make up a translational unit.  The idea is that
// the whole tiling can be replicated across the plane by placing
// a copy of the translational unit at every integer linear combination
// of the translation vectors.  In practice, we only draw at those
// linear combinations within some viewport.

#include <QStack>
#include "geometry/xform.h"
#include "settings/tristate.h"
#include "tile/tiling_data.h"

#define MAX_UNIQUE_TILE_INDEX 7

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;

typedef QVector<QTransform> Placements;

class TileGroup : public  QVector<QPair<TilePtr,PlacedTiles>>
{
public:
    bool containsTile(TilePtr fp);
    PlacedTiles & getTilePlacements(TilePtr fp);
};

// Translations to tile the plane. Two needed for two-dimensional plane.
// (Of course, more complex tiling exists with rotations and mirrors.)
// (They are not supported.)

class Tiling : public std::enable_shared_from_this<Tiling>
{
public:

    enum eTilingState
    {
        EMPTY,
        LOADED,
        MODIFIED
    };

    Tiling();
    ~Tiling();

    bool                isEmpty();

    bool                hasIntrinsicOverlaps();
    bool                hasTiledOverlaps();

    QString             getName()        const { return name; }
    QString             getDescription() const { return desc; }
    QString             getAuthor()      const { return author; }

    void                setName(QString n)               { name = n; }
    void                setDescription(QString descrip ) { desc = descrip; }
    void                setAuthor(QString auth)          { author = auth; }

    void                add(const PlacedTilePtr pf );   // Added tiles are put into a placedTile.
    void                add(TilePtr f, QTransform T );
    void                remove(PlacedTilePtr pf);

    // undo stack
    void                pushStack();
    bool                popStack();
    void                purgeStack() { undoStack.clear(); resetOverlaps(); }
    int                 stackSize()  { return undoStack.size(); }

    int                 numPlacements(TilePtr tile);
    Placements          getPlacements(TilePtr tile);
    QTransform          getFirstPlacement(TilePtr tile);
    QVector<TilePtr>    getUniqueTiles();
    TileGroup           regroupTiles();      // the map was deadly, it reordered

    void                setCanvasXform(const Xform & xf) { xf_canvas = xf; }
    Xform &             getCanvasXform()                 { return xf_canvas; }

    int                 getVersion();
    void                setVersion(int ver);

    eTilingState        getState();
    void                setState(eTilingState state);

    // Data
    const TilingData &  getData()  const          { return db; }
    TilingData &        getRWData(bool push)      { if (push) pushStack(); return db; }

    const PlacedTiles & getInTiling() const      { return getData().getInTiling(); }
    PlacedTiles &       getRWInTiling(bool push) { return getRWData(push).getRWInTiling(); }

    void setTranslationVectors(QPointF t1, QPointF t2) { db.setTranslationVectors(t1,t2); }

    void                resetOverlaps() { intrinsicOverlaps.reset();  tiledOverlaps.reset(); }

    // map operations
    MapPtr              createMapSingle();
    MapPtr              createMapFullSimple();
    MapPtr              createMapFull();
    MapPtr              debug_createFilledMap();
    MapPtr              debug_createProtoMap();

    QString             dump() const;

    static const QString defaultName;
    static int          refs;

protected:

private:
    int                 version;
    QString             name;
    QString             desc;
    QString             author;

    TilingData          db;

    QStack<TilingData>  undoStack;

    Xform               xf_canvas;

    eTilingState        state;
    Tristate            intrinsicOverlaps;
    Tristate            tiledOverlaps;
};

#endif

