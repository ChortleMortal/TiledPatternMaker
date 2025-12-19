#include "model/mosaics/writer_base.h"

WriterBase::WriterBase()
{
    refId = 0;
    vertex_ids.clear();
}

QString WriterBase::getVertexReference(VertexPtr ptr)
{
    int id =  vertex_ids.value(ptr);
    QString qs = QString(" reference=\"%1\"").arg(id);
    return qs;
}

void WriterBase::setVertexReference(int id, VertexPtr ptr)
{
    vertex_ids[ptr] = id;
}

bool WriterBase::hasReference(VertexPtr v)
{
    return vertex_ids.contains(v);
}

QString  WriterBase::id(int id)
{
    //qDebug() << "id=" << id;
    QString qs = QString(" id=\"%1\"").arg(id);
    return qs;
}

QString  WriterBase::nextId()
{
    return id(++refId);
}
