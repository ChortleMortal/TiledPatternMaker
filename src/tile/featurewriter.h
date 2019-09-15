#ifndef FEATUREWRITER_H
#define FEATUREWRITER_H

#include <QtCore>
#include <string>
#include "base/pugixml.hpp"
#include "geometry/Vertex.h"
#include "geometry/edgepoly.h"

using std::string;
using namespace pugi;

class FeatureWriter
{
public:
    FeatureWriter();

    void setEdgePoly(QTextStream & ts, EdgePoly & epoly);
    void setTransform(QTextStream & ts, QTransform T);

protected:
    void setVertex(QTextStream & ts,VertexPtr v, QString name);
    void setPoint(QTextStream & ts, QPointF pt, QString name);

    QString  id(int id);
    QString  nextId();
    int      getRef()  { return refId; }
    int      nextRef() { return ++refId; }
    QString  getVertexReference(VertexPtr ptr);
    void     setVertexReference(int id, VertexPtr ptr);
    bool     hasReference(VertexPtr map);

private:
    QMap<VertexPtr,int>   vertex_ids;
    int refId;
};

#endif // FEATUREWRITER_H
