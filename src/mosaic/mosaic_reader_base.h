#pragma once
#ifndef MOSAICREADERBASE_H
#define MOSAICREADERBASE_H

#include <QMap>
#if QT_VERSION < QT_VERSION_CHECK(6,5,0)
#include <memory>
#endif
#include "misc/pugixml.hpp"
#include "mosaic/mosaic_io_base.h"

using namespace pugi;

typedef std::shared_ptr<class Vertex> VertexPtr;

class MosaicReaderBase : public MosaicIOBase
{
public:
    MosaicReaderBase();

    bool        hasReference(xml_node & node);
    void        setVertexReference(xml_node & node, VertexPtr ptr);
    VertexPtr   getVertexReferencedPtr(xml_node & node);

protected:
    static QMap<int,VertexPtr>     vertex_ids;
};

#endif // MOSAICREADERBASE_H
