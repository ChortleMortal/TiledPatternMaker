#pragma once

#include "model/mosaics/mosaic_writer.h"

class MapEditor;

class MapEditorMapWriter : private MosaicWriter
{
    friend class TilingWriter;

public:
    MapEditorMapWriter();
    ~MapEditorMapWriter();

    bool writeXML(VersionedFile xfile, MapEditor *editor);

    using MosaicWriter::setCleanseMaps;
    using MosaicWriter::getUnverifiedMaps;
};

