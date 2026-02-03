#pragma once
#ifndef PROTOTYPE_H
#define PROTOTYPE_H

#include <QString>
#include <QTransform>
#include <QMutex>
#include <QMetaType>
#include <QDebug>

#include "sys/geometry/fill_region.h"

typedef std::shared_ptr<class Crop>             CropPtr;
typedef std::shared_ptr<class DCEL>             DCELPtr;
typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Tile>             TilePtr;
typedef std::shared_ptr<class Mosaic>           MosaicPtr;
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class DesignElement>    DELPtr;
typedef std::shared_ptr<class Prototype>        ProtoPtr;

typedef std::weak_ptr<class Prototype>          WeakProtoPtr;
typedef std::weak_ptr<class Mosaic>             WeakMosaicPtr;
typedef std::weak_ptr<class Tiling>             WeakTilingPtr;

Q_DECLARE_METATYPE(WeakProtoPtr)
Q_DECLARE_METATYPE(WeakTilingPtr)

class SystemViewController;

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
    Prototype(TilingPtr t, MosaicPtr m);
    Prototype(TilingPtr t);        // for use only by PrototypeMaker
    Prototype(MapPtr map);
    virtual ~Prototype();

    bool operator==(const Prototype & other);
    bool operator!=(const Prototype & other) { return !(*this == other); }

    // Maps
    void    createProtoMap(bool splash);
    void    wipeoutProtoMap();

    MapPtr getProtoMap(bool splash = false);
    MapPtr getExistingProtoMap()   { return _protoMap; }
    void   setProtoMap(MapPtr map) { _protoMap = map; }

    const DCELPtr & getDCEL();          // builds on demand
    const DCELPtr & getExistingDCEL()   { return _DCEL; }


    // Design Elements
    void              addDesignElement(const DELPtr & element);
    void              removeDesignElement(const DELPtr &element);
    QVector<DELPtr> &  getDesignElements() { return _designElements; }
    DELPtr  getDesignElement(int index);
    bool              containsDesignElement(DELPtr del);

    bool              hasContent()          { return (_designElements.size() > 0); }
    int               numDesignElements()   { return _designElements.size(); }

    MosaicPtr         getMosaic()           { return wMosaic.lock(); }
    void              setMosaic(MosaicPtr m){ wMosaic = m; }

    TilingPtr         getTiling()           { return _tiling; }
    void              replaceTiling(const TilingPtr & newTiling);
    void              exactReplaceTiling(const TilingPtr & newTiling);
    QList<TilePtr>    getTiles();
    TilePtr           getTile(const MotifPtr & motif);
    MotifPtr          getMotif(const TilePtr & tile);

    // Crop
    void    setCrop(CropPtr crop);
    void    resetCrop();
    CropPtr getCrop()                               { return _crop; }

    // distort
    bool    getDistortionEnable()                   { return distort; }
    void    enableDistortion(bool set)              { distort = set; }
    void    setDistortion(QTransform t)             { distortionTransform = t; }
    void    resetDistortion()                       { distortionTransform.reset(); }
    QTransform getDistortion()                      { return distortionTransform; }

    void    setViewController(SystemViewController * vc)  { viewController = vc; }

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

private:
    void    resetMotifMaps();
    void    buildMotifMaps();
    void    buildPrototypeMap(Placements &fillPlacements);
    void    analyze(TilingPtr newTiling);

    void    _createMap();

    // prototype data
    QVector<DELPtr>             _designElements;
    CropPtr                     _crop;

    // derived maps
    MapPtr                      _protoMap;
    DCELPtr                     _DCEL;

    TilingPtr                   _tiling;            // prototypes own tilings
    WeakMosaicPtr               wMosaic;            // mosaics own prototypes, si weak pointer
    SystemViewController *      viewController;     // can be changed

    QMutex                      dcelMutex;
    QMutex                      protoMutex;

    uint                        cleanseLevel;
    qreal                       cleanseSensitivity;

    bool                        distort;
    QTransform                  distortionTransform;
};

#endif

