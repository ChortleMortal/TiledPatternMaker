#ifndef TILING_WRITER
#define TILING_WRITER

#include "tile/placed_feature.h"
#include "geometry/xform.h"

class TilingWriter
{
public:
    TilingWriter(TilingPtr tiling) { this->tiling = tiling; };

    bool        writeTilingXML();
    void        writeTilingXML(QTextStream & out);     // also called when writing styles
    void        writeViewSettings(QTextStream & out);

protected:
    void    setEdgePoly(QTextStream & ts, EdgePoly & epoly);
    void    setVertex(QTextStream & ts,VertexPtr v, QString name);
    void    setPoint(QTextStream & ts, QPointF pt, QString name);

    bool    hasReference(VertexPtr map);
    QString getVertexReference(VertexPtr ptr);
    void    setVertexReference(int id, VertexPtr ptr);

    QString  id(int id);
    QString nextId();
    int     getRef()  { return refId; }

private:
    TilingPtr   tiling;

    QMap<VertexPtr,int>   vertex_ids;

    int         refId;
};

#endif

