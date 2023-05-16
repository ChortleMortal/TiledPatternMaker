#include "mosaic/mosaic_writer_base.h"

int MosaicWriterBase::refId = 0;

QMap<VertexPtr,int> MosaicWriterBase::vertex_ids;

MosaicWriterBase::MosaicWriterBase() : MosaicIOBase()
{
    refId = 0;
    vertex_ids.clear();
}

QString MosaicWriterBase::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

void MosaicWriterBase::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}

bool MosaicWriterBase::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}

QString  MosaicWriterBase::id(int id)
{
    //qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}

QString  MosaicWriterBase::nextId()
{
    return id(++refId);
}
