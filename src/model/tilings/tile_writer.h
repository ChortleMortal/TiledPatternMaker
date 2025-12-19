#pragma once
#ifndef TILE_WRITER_H
#define TILE_WRITER_H

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

protected:
    void setVertex(QTextStream & ts,VertexPtr v, QString name);
    void setPoint(QTextStream & ts, QPointF pt, QString name);

    WriterBase    twbase;
};

#endif // FEATUREWRITER_H
