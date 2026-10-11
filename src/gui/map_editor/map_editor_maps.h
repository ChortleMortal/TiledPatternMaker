#pragma once

#include <QVector>

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;
typedef std::shared_ptr<class Style>         StylePtr;
typedef std::shared_ptr<class Prototype>     ProtoPtr;
typedef std::shared_ptr<class TileMotif>     TileMotifPtr;
typedef std::shared_ptr<class Map>           MapPtr;

typedef std::weak_ptr<class Style>          WeakStylePtr;
typedef std::weak_ptr<class Prototype>      WeakProtoPtr;
typedef std::weak_ptr<class TileMotif>      WeakTileMotifPtr;
typedef std::weak_ptr<class Tiling>         WeakTilingPtr;

typedef std::shared_ptr<class MapEditorMap> MapedMapPtr;

typedef QVector<TileMotifPtr>               TileMotifSet;

class MapEditorMap
{
public:

    enum class MapType
    {
        None = 0,
        Style,
        Prototype,
        Motif,
        MotifUnit,
        TilingUnit,
        TilingFull,
        File,
        FileMotif,
        Created,
        Composite,
        CompositeMotif,
        Crop,
    };

    MapEditorMap(MapPtr map, MapType mtype);

    MapPtr   getMap()       { return map; }
    MapType  getType()      { return mtype; }

protected:
    MapPtr          map;
    MapType         mtype;
};

class StyleMap : public MapEditorMap
{
    using MapType = MapEditorMap::MapType;

public:
    StyleMap(MapPtr map, StylePtr style);

    StylePtr getStyle()     { return wsp.lock(); }

private:
    WeakStylePtr    wsp;
};

class ProtoMap : public MapEditorMap
{
    using MapType = MapEditorMap::MapType;

public:
    ProtoMap(MapPtr map, ProtoPtr proto);

    ProtoPtr getPrototype() { return wpp.lock(); }

private:
    WeakProtoPtr    wpp;
};

class TimoMap : public MapEditorMap
{
    using MapType = MapEditorMap::MapType;

public:
    TimoMap(MapPtr map, ProtoPtr proto, TileMotifSet timos, MapType mtype);

    ProtoPtr     getPrototype() { return wpp.lock(); }
    TileMotifSet getTileMotifs();

private:
    WeakProtoPtr              wpp;
    QVector<WeakTileMotifPtr> wtimos;
};

class TilingMap : public MapEditorMap
{
    using MapType = MapEditorMap::MapType;

public:
    TilingMap(MapPtr map, TilingPtr tiling, MapType mtype);

    TilingPtr   getTiling()     { return wtp.lock(); }

private:
    WeakTilingPtr   wtp;
    WeakProtoPtr    wpp;
};

