#pragma once

#include <QTextStream>
#include "model/mosaics/writer_base.h"
#include "sys/geometry/edge_poly.h"
#include "sys/geometry/vertex.h"
#include "sys/sys/pugixml.hpp"

using std::string;
using namespace pugi;

class TileWriter
{
public:
    TileWriter();

    void setEdgePoly(QTextStream & ts, const EdgePoly & epoly);
    void setTransform(QTextStream & ts, QTransform T);

private:
    void setVertex(QTextStream & ts,VertexPtr v, QString name);
    void setPoint(QTextStream & ts, QPointF pt, QString name);

    WriterBase    twbase;
};
