#pragma once
#ifndef TILE_READER_H
#define TILE_READER_H

#include <string>
#include "model/mosaics/reader_base.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/vertex.h"
#include "sys/sys/pugixml.hpp"

class ReaderBase;

using std::string;
using namespace pugi;

class TileReader
{
public:
    TileReader(ReaderBase * mrbase);

    EdgeSet     getEdgeSet(xml_node & node, bool legacyFlipConcave = true);
    QTransform  getTransform(xml_node & node);

protected:
    VertexPtr   getVertex(xml_node & node);
    QPointF     getPoint(xml_node & node);

    ReaderBase *   mrbase;
};

#endif // FEATUREREADER_H
