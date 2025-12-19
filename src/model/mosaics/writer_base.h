#pragma once
#ifndef WRITERBASE_H
#define WRITERBASE_H

#include <QString>
#include <QMap>
#if (QT_VERSION < QT_VERSION_CHECK(6,5,0))
#include <memory>
#endif

typedef std::shared_ptr<class Vertex> VertexPtr;

class WriterBase
{
public:
    WriterBase();

    QString getVertexReference(VertexPtr ptr);
    void    setVertexReference(int id, VertexPtr ptr);
    bool    hasReference(VertexPtr map);

    QString id(int id);
    QString nextId();
    int     getRef()  { return refId; }
    int     nextRef() { return ++refId; }

    int     refId;

    QMap<VertexPtr,int> vertex_ids;
};

#endif // MOSAICWRITERBASE_H
