#ifndef MAP_EDITOR_MAP_WRITER
#define MAP_EDITOR_MAP_WRITER

#include "mosaic/mosaic_writer.h"
#include "enums/emapeditor.h"

typedef std::shared_ptr<class MapEditorView>    MapedViewPtr;

class MapEditorView;

class MapEditorMapWriter : private MosaicWriter
{
    friend class TilingWriter;

public:
    MapEditorMapWriter(MapedViewPtr  view);
    ~MapEditorMapWriter();

    bool writeXML(QString fileName, MapPtr map, eMapEditorMapType mapType);

private:
    MapedViewPtr meView;
};

#endif
