#ifndef MOSAICREADERBASE_H
#define MOSAICREADERBASE_H

#include <memory>
#include <QMap>
#include "misc/pugixml.hpp"

using namespace pugi;

typedef std::shared_ptr<class Vertex> VertexPtr;

class MosaicReaderBase
{
public:
    MosaicReaderBase();

    bool        hasReference(xml_node & node);
    void        setVertexReference(xml_node & node, VertexPtr ptr);
    VertexPtr   getVertexReferencedPtr(xml_node & node);

protected:
    static QMap<int,VertexPtr>     vertex_ids;
    static int nRefrCnt;

};

#endif // MOSAICREADERBASE_H
