#pragma once

#include <QRectF>
#include <QString>
#include <QTransform>

typedef std::shared_ptr<class Motif>        MotifPtr;
typedef std::shared_ptr<class Prototype>    ProtoPtr;
typedef std::shared_ptr<class Tile>         TilePtr;
typedef std::shared_ptr<class Tiling>       TilingPtr;
typedef std::shared_ptr<class TileMotif>    TileMotifPtr;

typedef std::weak_ptr<class Tiling>         WeakTilingPtr;
typedef std::weak_ptr<class TileMotif>      WeakTileMotifPtr;

////////////////////////////////////////////////////////////////////////////
//
// DesignElement.java
//
// A DesignElement is the core of the process of building a finished design.
// It's a Tile together with a Motif.  The tile comes from the
// tile library and will be used to determine where to place copies of the
// Motif, which is designed by the user.

class TileMotif
{
public:
    TileMotif(const TilingPtr & tiling);
    TileMotif(const TilingPtr & tiling, const TilePtr & tile, const MotifPtr & motif);
    TileMotif(const TilingPtr & tiling, const TilePtr & tile);
    TileMotif(const TilingPtr & tiling, const TileMotifPtr & dep);
    TileMotif(const TilingPtr & tiling, const TileMotif & other);
    ~TileMotif();

    TilePtr     getTile()   const   { return tile; }
    MotifPtr    getMotif()  const   { return motif; }
    TilingPtr   getTiling() const   { return tiling.lock(); }

    void        exactReplaceTile(const TilePtr & tile);
    void        replaceModifiedTile(const TilePtr & tile);

    void        setMotif(const MotifPtr & motif);
    bool        validMotifRegularity();
    void        createMotifFromTile();
    void        recreateMotifWhenRgularityChanged();
    void        recreateMotifFromChangedTile();

    QRectF      boundingRect() const;    // bounding box of the tile's points and the motif's map vertices

    QString     toString();
    void        describe();

    static int  refs;

protected:
    TilePtr       tile;
    MotifPtr	  motif;
    WeakTilingPtr tiling;  // 27JAN26 - if this was done earlier would have simplified code
};

