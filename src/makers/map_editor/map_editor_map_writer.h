#pragma once
#ifndef MAP_EDITOR_MAP_WRITER
#define MAP_EDITOR_MAP_WRITER

#include "mosaic/mosaic_writer.h"
#include "enums/emapeditor.h"


class MapEditorView;

class MapEditorMapWriter : private MosaicWriter
{
    friend class TilingWriter;

public:
    MapEditorMapWriter(MapEditorView * view);
    ~MapEditorMapWriter();

    bool writeXML(QString fileName, MapPtr map, eMapEditorMapType mapType);

private:
    MapEditorView * meView;
};

#endif
