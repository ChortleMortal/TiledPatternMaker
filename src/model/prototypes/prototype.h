#pragma once
#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#include <QString>
#include <QTransform>
#include <QMutex>
#include <QMetaType>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;

typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class Mosaic>             WeakMosaicPtr;
typedef std::weak_ptr<class Tiling>             WeakTilingPtr;

Q_DECLARE_METATYPE(WeakProtoPtr)

class ViewController;

////////////////////////////////////////////////////////////////////////////
//
// Prototype.java
//
// The complete information needed to build a pattern: the tiling and
// a mapping from features to figures.  Prototype knows how to turn
// this information into a finished design, returned as a Map.

// DAC: The implementation requires 'figures' (now designElements) to be an ordered list.
// Taprats uses a hash based on an algorithm which presumably provided an order.
// Generally I use QMap to replace the hashes but its
// order would be randomly based on an address in memory, so instead I have
// created an array of the feature and figure pointers1

class Prototype
{
    friend class PrototypeMaker;

public:
    explicit Prototype(TilingPtr t, MosaicPtr m);
    explicit Prototype(TilingPtr t);        // for use only by PrototypeMaker
    explicit Prototype(MapPtr map);
    virtual ~Prototype();

    bool    operator==(const Prototype & other);

    // Maps
    void    createProtoMap(bool splash);
    void    wipeoutProtoMap();

    MapPtr  getProtoMap(bool splash = false);
    MapPtr  getExistingProtoMap()   { return _protoMap; }

    DCELPtr getDCEL();          // builds on demand
    DCELPtr getExistingDCEL()  { return _dcel; }

    void    resetMotifMaps();
    void    buildMotifMaps();

    // Design Elements
    void              addDesignElement(const DesignElementPtr & element);
    void              removeDesignElement(const DesignElementPtr &element);
    QVector<DesignElementPtr> &  getDesignElements() { return _designElements; }
  //DesignElementPtr  getDesignElement(const TilePtr & tile);
    DesignElementPtr  getDesignElement(int index);
    bool              containsDesignElement(DesignElementPtr del);

    bool              hasContent()          { return (_designElements.size() > 0); }
    int               numDesignElements()   { return _designElements.size(); }

    MosaicPtr         getMosaic()           { return wMosaic.lock(); }
    void              setMosaic(MosaicPtr m){ wMosaic = m; }

    TilingPtr         getTiling()           { return _tiling; }
    void              replaceTiling(const TilingPtr & newTiling);
    QList<TilePtr>    getTiles();
    TilePtr           getTile(const MotifPtr & motif);
    MotifPtr          getMotif(const TilePtr & tile);

    // Crop
    void    setCrop(CropPtr crop);
    void    resetCrop();
    CropPtr getCrop()                       { return _crop; }

    void    setViewController(ViewController * vc) { viewController = vc; }
    void    setCleanseLevel(uint level)            { cleanseLevel = level; }
    void    setCleanseSensitivity(qreal val)       { cleanseSensitivity = val; }

    void    walk();
    void    dump();
    void    dumpMotifs();
    QString info() const;

    static int refs;

protected:

    void    analyze(TilingPtr newTiling);

private:
    void    _createMap();

    // prototype data
    QVector<DesignElementPtr>   _designElements;
    CropPtr                     _crop;

    // derived maps
    MapPtr                      _protoMap;
    DCELPtr                     _dcel;

    TilingPtr                   _tiling;            // prototypes own tilings
    WeakMosaicPtr                wMosaic;           // mosaics own prototypes, si weak pointer
    class ViewController      * viewController;     // can be changed

    QMutex                      dcelMutex;
    QMutex                      protoMutex;

    uint                        cleanseLevel;
    qreal                       cleanseSensitivity;
};

#endif

