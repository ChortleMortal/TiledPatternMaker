#pragma once

#include "model/tilings/backgroundimage.h"
#include <QTextStream>
#include <QMap>
#include "sys/geometry/vertex.h"
#include "sys/geometry/xform.h"
#include "sys/sys/versioning.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class Vertex>           VertexPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

typedef QVector<EdgePtr> EdgeSet;

class EdgePoly;

class TilingWriter
{
    friend class TilingMaker;
    friend class TilingBMPGenerator;
    friend class MosaicReader;

public:
    bool    writeTilingXML(VersionedFile vfile, TilingPtr tiling);

    static void writeBackgroundImage(QTextStream & out, const BkgdRecord & rec);

protected:
    void    writeXML(QTextStream & out);     // also called when writing styles
    void    writeViewSettings(QTextStream & out);
    void    writeConstructionLines(QTextStream & out);

    void    setEdgeSet(QTextStream & ts, const EdgeSet & eset);
    void    setVertex(QTextStream & ts,VertexPtr v, QString name);
    void    setPoint(QTextStream & ts, QPointF pt, QString name);

    bool    hasReference(VertexPtr map);
    QString getVertexReference(VertexPtr ptr);
    void    setVertexReference(int id, VertexPtr ptr);

    void    procesToolkitGeoLayer(QTextStream & ts, const Xform & xf, int zlevel);

    QString id(int id);
    QString nextId();
    int     getRef()  { return refId; }

private:
    TilingWriter() {}

    TilingPtr   tiling;

    QMap<VertexPtr,int>   vertex_ids;
    int         refId;
};

