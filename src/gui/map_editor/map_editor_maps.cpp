#include <QDebug>

#include "gui/map_editor/map_editor_maps.h"
#include "sys/geometry/map.h"

MapEditorMap::MapEditorMap(MapPtr map, MapType mtype)
{
    this->map   = map;
    this->mtype = mtype;
}

StyleMap::StyleMap(MapPtr map, StylePtr style) : MapEditorMap(map,MapType::Style)
{
    wsp = style;
}

ProtoMap::ProtoMap(MapPtr map, ProtoPtr proto) : MapEditorMap(map,MapType::Prototype)
{
    wpp = proto;
}

TimoMap::TimoMap(MapPtr map, ProtoPtr proto, TileMotifSet timos, MapType mtype) : MapEditorMap(map,mtype)

{
    wpp = proto;
    for (auto & timo : timos)
    {
        WeakTileMotifPtr wtimo = timo;
        wtimos.push_back(wtimo);
    }
}

TileMotifSet TimoMap::getTileMotifs()
{
    TileMotifSet set;
    for (auto & wtimo : wtimos)
    {
        TileMotifPtr timo = wtimo.lock();
        if (timo)
            set.push_back(timo);
    }
    return set;
}


TilingMap::TilingMap(MapPtr map, TilingPtr tiling, MapType mtype) : MapEditorMap(map,mtype)
{
    wtp = tiling;
}

