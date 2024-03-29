#pragma once
#ifndef MAP_EDITOR_MAP_LOADER_H
#define MAP_EDITOR_MAP_LOADER_H

#include "mosaic/mosaic_reader.h"
#include "enums/emapeditor.h"

class MapEditorMapLoader : private MosaicReader
{
public:
    MapEditorMapLoader();
    ~MapEditorMapLoader();

    MapPtr              loadMosaicMap(QString fileName);

    Xform               getXform ()     { return _xf; }
    eMapEditorMapType   getType()       { return maptype; }
    QSize               getViewSize()   { return _viewSize; }
    BkgdImagePtr        getBackground() { return _bip; }

private:
    Xform               _xf;
    eMapEditorMapType   maptype;
};

#endif
