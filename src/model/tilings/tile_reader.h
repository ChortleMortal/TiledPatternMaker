#pragma once
#ifndef TILE_READER_H
#define TILE_READER_H

#include <string>
#include "sys/sys/pugixml.hpp"
#include "sys/geometry/vertex.h"
#include "sys/geometry/edgepoly.h"
#include "model/mosaics/mosaic_reader_base.h"

typedef std::shared_ptr<MosaicReaderBase> MRBasePtr;

using std::string;
using namespace pugi;

class TileReader
{
public:
    TileReader();
    TileReader(MRBasePtr base);

    EdgePoly    getEdgePoly(xml_node & node, bool legacyFlip = true);
    QTransform  getTransform(xml_node & node);

protected:
    VertexPtr   getVertex(xml_node & node);
    QPointF     getPoint(xml_node & node);

    MRBasePtr   base;
};

#endif // FEATUREREADER_H
