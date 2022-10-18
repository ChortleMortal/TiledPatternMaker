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

#ifndef TILING
#define TILING

#include <QStack>
#include "geometry/xform.h"
#include "settings/tristate.h"
#include "tile/tiling_data.h"

#define MAX_UNIQUE_TILE_INDEX 7

typedef std::shared_ptr<class Tile>          TilePtr;
typedef std::shared_ptr<class PlacedTile>    PlacedTilePtr;
typedef std::shared_ptr<class Map>           MapPtr;


class TileGroup : public  QVector<QPair<TilePtr,QVector<PlacedTilePtr>>>
{
public:
    bool containsTile(TilePtr fp);
    QVector<PlacedTilePtr> & getPlacements(TilePtr fp);
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
    Tiling(QString name, QPointF t1, QPointF t2);
    ~Tiling();

    bool        isEmpty();

    bool        hasIntrinsicOverlaps();
    bool        hasTiledOverlaps();

    QString     getName()        const { return name; }
    QString     getDescription() const { return desc; }
    QString     getAuthor()      const { return author; }

    void        setName(QString n)               { name = n; }
    void        setDescription(QString descrip ) { desc = descrip; }
    void        setAuthor(QString auth)          { author = auth; }

    // Feature management.
    // Added features are embedded into a placedTile.
    void        add(const PlacedTilePtr pf );
    void        add(TilePtr f, QTransform T );
    void        remove(PlacedTilePtr pf);

    // undo stack
    void        pushStack();
    bool        popStack();
    void        purgeStack() { undoStack.clear(); }
    int         stackSize()  { return undoStack.size(); }

    int                       numPlacements(TilePtr fp);
    QVector<QTransform>       getPlacements(TilePtr fp);
    QTransform                getPlacement(TilePtr fp, int index);

    QVector<TilePtr>          getUniqueTiles();
    TileGroup                 regroupTiles();      // the map was deadly, it reordered

    void        setCanvasXform(const Xform & xf) { canvasXform = xf; }
    Xform &     getCanvasXform()                 { return canvasXform; }

    int         getVersion();
    void        setVersion(int ver);

    eTilingState getState();
    void         setState(eTilingState state);

    // Data

    const TilingData & getData() { return db; }
    TilingData & getDataAccess() { pushStack(); state = MODIFIED; return db; }

    // map operations
    MapPtr  createMapSingle();
    MapPtr  createMapFullSimple();
    MapPtr  createMapFull();

    QString     dump() const;

    static const QString defaultName;
    static int  refs;

protected:

private:
    int             version;
    QString         name;
    QString         desc;
    QString         author;
    TilingData      db;

    QStack<TilingData> undoStack;

    Xform           canvasXform;

    eTilingState    state;
    Tristate        intrinsicOverlaps;
    Tristate        tiledOveraps;
};

#endif

