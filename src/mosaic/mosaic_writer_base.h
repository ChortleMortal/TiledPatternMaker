#ifndef MOSAICWRITERBASE_H
#define MOSAICWRITERBASE_H

#include <memory>
#include <QString>
#include <QMap>

typedef std::shared_ptr<class Vertex> VertexPtr;

class MosaicWriterBase
{
public:
    MosaicWriterBase();

protected:
    // writer methods
    QString getVertexReference(VertexPtr ptr);
    void    setVertexReference(int id, VertexPtr ptr);
    bool    hasReference(VertexPtr map);

    QString id(int id);
    QString nextId();
    int     getRef()  { return refId; }
    int     nextRef() { return ++refId; }

    static int refId;

    static QMap<VertexPtr,int> vertex_ids;
};

#endif // MOSAICWRITERBASE_H
