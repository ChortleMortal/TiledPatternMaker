#pragma once

#include <QColor>

#include "gui/map_editor/map_editor_maps.h"

class MapEditorLayer
{
    using MapType = MapEditorMap::MapType;

public:

    enum class LayerId
    {
        None        = 0x00,
        L1          = 0x01,
        L2          = 0x02,
        L3          = 0x04,
        Composite   = 0x08
    };

    MapEditorLayer();

    void init(LayerId id, QColor color)    { this->id = id; this->color = color; }

    void setLayer(MapPtr map, StylePtr style);
    void setLayer(MapPtr map, ProtoPtr proto);
    void setLayer(MapPtr map, ProtoPtr proto, TileMotifSet timos, MapType mtype);
    void setLayer(MapPtr map, TilingPtr tiling, MapType mtype);
    void setLayer(MapPtr map, MapType mtype);

    void reset();

    MapPtr          getMapedLayerMap()  const { if (working) return working->getMap(); else return nullptr; }
    MapType         getLayerMapType()   const { if (working) return working->getType();else return MapType::None; }

    LayerId         getId()             const { return id; }
    QColor          getColor()          const { return color; }
    MapedMapPtr     getSource()         const { return  source; }
    MapedMapPtr     getWorking()        const { return  working; }

protected:

private:
    MapedMapPtr     source;
    MapedMapPtr     working;        // the working map

    LayerId         id;
    QColor          color;
};

