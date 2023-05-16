#include <QDebug>
#include "mosaic/mosaic_reader_base.h"

QMap<int,VertexPtr>   MosaicReaderBase::vertex_ids;
int MosaicReaderBase::nRefrCnt = 0;

MosaicReaderBase::MosaicReaderBase() : MosaicIOBase()
{
}

bool MosaicReaderBase::hasReference(xml_node & node)
{
    xml_attribute ref;
    ref = node.attribute("reference");
    return (ref);
}

void MosaicReaderBase::setVertexReference(xml_node & node, VertexPtr ptr)
{
    xml_attribute id;
    id = node.attribute("id");
    if (id)
    {
        int i = id.as_int();
        vertex_ids[i] = ptr;
#ifdef DEBUG_REFERENCES
        qDebug() << "set ref vertex:" << i;
#endif
    }
}

VertexPtr MosaicReaderBase::getVertexReferencedPtr(xml_node & node)
{
    VertexPtr retval;
    xml_attribute ref;
    ref = node.attribute("reference");
    if (ref)
    {
        int id = ref.as_int();
#ifdef DEBUG_REFERENCES
        qDebug() << "using reference" << id;
#endif
        retval = VertexPtr(vertex_ids[id]);
        if (!retval)
        {
            qCritical() << "reference id:" << id << "- NOT FOUND";
        }
    }
    return retval;
}
