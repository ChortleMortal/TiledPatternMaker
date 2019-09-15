#ifndef FEATUREREADER_H
#define FEATUREREADER_H

#include <QtCore>
#include <string>
#include "base/pugixml.hpp"
#include "geometry/Vertex.h"
#include "geometry/edgepoly.h"

using std::string;
using namespace pugi;

class FeatureReader
{
public:
    FeatureReader();

    EdgePoly    getEdgePoly(xml_node & node);
    QTransform  getTransform(xml_node & node);

protected:
    VertexPtr   getVertex(xml_node & node);
    QPointF     getPoint(xml_node & node);

    bool        hasReference(xml_node & node);
    void        setVertexReference(xml_node & node, VertexPtr ptr);
    VertexPtr   getVertexReferencedPtr(xml_node & node);

private:
    QMap<int,VertexPtr>     vertex_ids;

    int vOrigCnt;
    int vRefrCnt;
};

#endif // FEATUREREADER_H
