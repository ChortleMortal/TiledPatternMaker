#pragma once

#include "gui/map_editor/map_editor_maps.h"
#include "model/mosaics/mosaic_reader.h"

class MapEditorMapLoader : private MosaicReader
{
    using MapType = MapEditorMap::MapType;

public:
    MapEditorMapLoader();
    ~MapEditorMapLoader();

    MapPtr          loadMosaicMap(VersionedFile xfile);

    Xform           getXform ()     { return _xf; }
    MapType         getType()       { return maptype; }
    QSize           getViewSize()   { return _viewSize; }

private:
    Xform           _xf;
    MapType         maptype;
};
