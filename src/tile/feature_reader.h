#ifndef FEATUREREADER_H
#define FEATUREREADER_H

#include <string>
#include "misc/pugixml.hpp"
#include "geometry/vertex.h"
#include "geometry/edgepoly.h"
#include "mosaic/mosaic_reader_base.h"

using std::string;
using namespace pugi;

class FeatureReader : public MosaicReaderBase
{
public:
    FeatureReader();

    EdgePoly    getEdgePoly(xml_node & node);
    QTransform  getTransform(xml_node & node);

protected:
    VertexPtr   getVertex(xml_node & node);
    QPointF     getPoint(xml_node & node);
};

#endif // FEATUREREADER_H
