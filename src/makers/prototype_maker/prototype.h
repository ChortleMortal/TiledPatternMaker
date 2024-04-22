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
typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;

typedef std::weak_ptr<class Prototype> WeakProtoPtr;
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
// created an array of the feature and figure pointers

class Prototype
{
public:
    explicit Prototype(TilingPtr t);
    explicit Prototype(MapPtr map);
    virtual ~Prototype();

    bool    operator==(const Prototype & other);

    void    createProtoMap();
    void    wipeoutProtoMap();

    MapPtr  getProtoMap();
    MapPtr  getExistingProtoMap() { return _protoMap; }

    DCELPtr getDCEL();

    void    replaceTiling(const TilingPtr & newTiling);
    void    addElement(const DesignElementPtr & element);
    void    removeElement(const DesignElementPtr &element);

    bool    hasContent() { return (designElements.size() > 0); }

    int     numDesignElements() { return designElements.size(); }

    TilingPtr         getTiling()   { return tiling; }
    QList<TilePtr>    getTiles();
    TilePtr           getTile(const MotifPtr & motif);

    MotifPtr          getMotif(const TilePtr & tile);

    DesignElementPtr  getDesignElement(const TilePtr & tile);
    DesignElementPtr  getDesignElement(int index);
    bool              contains(DesignElementPtr del);
    QVector<DesignElementPtr> &  getDesignElements() { return designElements; }

    void    setCrop(CropPtr crop);
    void    resetCrop();
    CropPtr getCrop() { return _crop; }

    void    setViewController(ViewController * vc) { viewController = vc; }

    void    walk();
    void    dump();
    void    dumpMotifs();
    QString info() const;

    static int refs;

protected:
    void    createMap();
    void    analyze(TilingPtr newTiling);

private:
    TilingPtr                   tiling;
    QVector<DesignElementPtr>   designElements;
    MapPtr                      _protoMap;
    CropPtr                     _crop;
    DCELPtr                     _dcel;

    class ControlPanel        * panel;
    class Configuration       * config;
    class ViewController      * viewController;

    QMutex                      dcelMutex;
    QMutex                      protoMutex;
};

#endif

