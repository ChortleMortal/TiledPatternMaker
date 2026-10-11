#pragma once

#include <QString>
#include <QTransform>
#include <QMutex>
#include <QMetaType>
#include <QDebug>
#include <QPair>

#include "sys/sys.h"
#include "sys/geometry/fill_region.h"

typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class TileMotif>        TileMotifPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;

typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class Mosaic>             WeakMosaicPtr;
typedef std::weak_ptr<class Tiling>             WeakTilingPtr;

typedef QVector<TileMotifPtr>                   TileMotifSet;

Q_DECLARE_METATYPE(WeakProtoPtr)
Q_DECLARE_METATYPE(WeakTilingPtr)

////////////////////////////////////////////////////////////////////////////
//
// Prototype.java
//
// The complete information needed to build a pattern: the tiling and
// a mapping from features to figures.  Prototype knows how to turn
// this information into a finished design, returned as a Map.

// DAC: The implementation requires 'figures' (now TileMotifs) to be an ordered list.
// Taprats uses a hash based on an algorithm which presumably provided an order.
// Generally I use QMap to replace the hashes but its
// order would be randomly based on an address in memory, so instead I have
// created an array of the feature and figure pointers1

class Prototype
{
    friend class MosaicReader;
    friend class MotifMaker;
    friend void  Sys::render(eRenderType rtype);

public:
    Prototype(TilingPtr t, MosaicPtr m);
    Prototype(TilingPtr t);        // for use only by otifMaker
    Prototype(MapPtr map);
    virtual ~Prototype();

    bool operator==(const Prototype & other);
    bool operator!=(const Prototype & other) { return !(*this == other); }

    // Maps
    void            wipeoutProtoMap();
    MapPtr          getProtoMap();       // builds on demand
    MapPtr          getExistingProtoMap()   { return _protoMap; }
    void            setProtoMap(MapPtr map);
    const DCELPtr & getDCEL();                              // builds on demand
    const DCELPtr & getExistingDCEL()       { return _DCEL; }

    // TileMotifs
    void              addTileMotif(const TileMotifPtr & tileMotif);
    void              removeTileMotif(const TileMotifPtr & tileMotif);
    TileMotifSet    & getTileMotifs()       { return _tileMotifSet; }
    TileMotifPtr      getTileMotif(int index);
    bool              containsTileMotif(TileMotifPtr tileMotif);

    bool              hasContent();
    int               numTileMotifs()       { return _tileMotifSet.size(); }

    MosaicPtr         getMosaic()           { return wMosaic.lock(); }
    void              setMosaic(MosaicPtr m){ wMosaic = m; }

    class Canvas *    getCanvas();          // the mosaic's canvas

    TilingPtr         getTiling()           { return _tiling; }
    void              replaceTiling(const TilingPtr & newTiling);
    void              recreateFromTiling();

    void              exactReplaceTiling(const TilingPtr & newTiling);
    QList<TilePtr>    getTiles();
    TilePtr           getTile(const MotifPtr & motif);
    MotifPtr          getMotif(const TilePtr & tile);

    // Crop
    void    setCrop(CropPtr crop);
    void    resetCrop();
    CropPtr getCrop()                               { return _crop; }

    void    setCleanseLevel(uint level)             { cleanseLevel = level; }
    void    setCleanseSensitivity(qreal val)        { cleanseSensitivity = val; }
    uint    getCleanseLevel()                       { return cleanseLevel; }
    qreal   getCleanseSensitivity()                 { return cleanseSensitivity; }

    void    walk();
    void    dump();
    void    dumpMotifs();
    QString info() const;

    static int refs;

protected:
    void    analyze(TilingPtr newTiling);

private:
    void    splashCreateProtoMap();
    void    createProtoMap();
    void    buildMotifMaps();
    void    _buildPrototypeMap(Placements &fillPlacements);

    // pairs up leftover (unmatched) old TileMotifs and new tiles that are probably
    // "the same tile, edited" rather than a true add+delete, by shape closeness -
    // see the .cpp for the algorithm.
    QVector<QPair<TileMotifPtr,TilePtr>> pairModifiedTiles(const QVector<TileMotifPtr> & unmatchedOld,
                                                             const QVector<TilePtr> & unmatchedNew);

    // prototype data
    TileMotifSet                _tileMotifSet;
    CropPtr                     _crop;

    // derived maps
    MapPtr                      _protoMap;
    DCELPtr                     _DCEL;
    bool                        _dcelFailed = false;

    TilingPtr                   _tiling;            // prototypes own tilings
    WeakMosaicPtr               wMosaic;            // mosaics own prototypes, si weak pointer

    QMutex                      dcelMutex;
    QMutex                      protoMutex;

    uint                        cleanseLevel;
    qreal                       cleanseSensitivity;
};


