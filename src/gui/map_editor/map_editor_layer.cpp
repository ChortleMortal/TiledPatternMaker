#include <QDebug>

#include "gui/map_editor/map_editor_layer.h"
#include "gui/top/view_controller.h"
#include "model/settings/configuration.h"
#include "sys/geometry/map.h"
#include "sys/tiledpatternmaker.h"

 using MapType = MapEditorMap::MapType;

//////////////////////////////////////////////////////////////////////
///
///  MapEditor Layer
///
//////////////////////////////////////////////////////////////////////

MapEditorLayer::MapEditorLayer()
{
    reset();
}

void MapEditorLayer::setLayer(MapPtr map, StylePtr style)
{
    qDebug() << "Using map" << map->summary();

    source = make_shared<StyleMap>(map,style);

    if (Sys::config->mapedLoadCopies)
    {
        auto wmap = map->recreate();
        working  = make_shared<StyleMap>(wmap,style);
    }
    else
    {
        working = source;
    }
}

void MapEditorLayer::setLayer(MapPtr map, ProtoPtr proto)
{
    qDebug() << "Using map" << map->summary();

    source = make_shared<ProtoMap>(map,proto);

    if (Sys::config->mapedLoadCopies)
    {
        auto wmap = map->recreate();
        working   = make_shared<ProtoMap>(wmap,proto);
    }
    else
    {
        working = source;
    }
}

void MapEditorLayer::setLayer(MapPtr map, ProtoPtr proto, TileMotifSet timos, MapType mtype)
{
    qDebug() << "Using map" << map->summary();

    source = make_shared<TimoMap>(map,proto,timos,mtype);

    if (Sys::config->mapedLoadCopies)
    {
        auto wmap = map->recreate();
        working = make_shared<TimoMap>(wmap,proto,timos,mtype);
    }
    else
    {
        working = source;
    }
}

void MapEditorLayer::setLayer(MapPtr map, TilingPtr tiling, MapType mtype)
{
    qDebug() << "Using map" << map->summary();

    source = make_shared<TilingMap>(map,tiling,mtype);

    if (Sys::config->mapedLoadCopies)
    {
        auto wmap = map->recreate();
        working = make_shared<TilingMap>(wmap,tiling,mtype);
    }
    else
    {
        working = source;
    }
}

void MapEditorLayer::setLayer(MapPtr map, MapType mtype)
{
    qDebug() << "Using map" << map->summary();

    source = make_shared<MapEditorMap>(map,mtype);

    if (Sys::config->mapedLoadCopies)
    {
        auto wmap = map->recreate();
        working   = make_shared<MapEditorMap>(wmap,mtype);
    }
    else
    {
        working = source;
    }
}

void MapEditorLayer::reset()
{
    working.reset();
    source.reset();
}

