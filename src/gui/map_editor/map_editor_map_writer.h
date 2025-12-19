#pragma once
#ifndef MAP_EDITOR_MAP_WRITER
#define MAP_EDITOR_MAP_WRITER

#include "model/mosaics/mosaic_writer.h"

class MapEditorDb;

class MapEditorMapWriter : private MosaicWriter
{
    friend class TilingWriter;

public:
    MapEditorMapWriter();
    ~MapEditorMapWriter();

    bool writeXML(VersionedFile xfile, MapEditorDb * db);
};

#endif
