#ifndef TILING_WRITER
#define TILING_WRITER

#include "geometry/vertex.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImgPtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;

class EdgePoly;

class TilingWriter
{
    friend class TilingManager;

public:
    bool    writeTilingXML();
    void    writeTilingXML(QTextStream & out);     // also called when writing styles
    void    writeViewSettings(QTextStream & out);
    static void writeBackgroundImage(QTextStream & out, BkgdImgPtr bkgd);

protected:
    void    setEdgePoly(QTextStream & ts, EdgePoly & epoly);
    void    setVertex(QTextStream & ts,VertexPtr v, QString name);
    void    setPoint(QTextStream & ts, QPointF pt, QString name);

    bool    hasReference(VertexPtr map);
    QString getVertexReference(VertexPtr ptr);
    void    setVertexReference(int id, VertexPtr ptr);

    QString id(int id);
    QString nextId();
    int     getRef()  { return refId; }

private:
    TilingWriter(TilingPtr tiling) { this->tiling = tiling; };

    TilingPtr   tiling;

    QMap<VertexPtr,int>   vertex_ids;
    int         refId;
};

#endif

