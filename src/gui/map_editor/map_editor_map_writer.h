#pragma once
#ifndef MAP_EDITOR_MAP_WRITER
#define MAP_EDITOR_MAP_WRITER

#include "model/mosaics/mosaic_writer.h"
#include "sys/enums/emapeditor.h"


class MapEditorView;

class MapEditorMapWriter : private MosaicWriter
{
    friend class TilingWriter;

public:
    MapEditorMapWriter(MapEditorView * view);
    ~MapEditorMapWriter();

    bool writeXML(VersionedFile xfile, MapPtr map, eMapEditorMapType mapType);

private:
    MapEditorView * meView;
};

#endif
