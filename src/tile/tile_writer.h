#pragma once
#ifndef TILE_WRITER_H
#define TILE_WRITER_H

#include <QTextStream>
#include "mosaic/mosaic_writer_base.h"
#include "misc/pugixml.hpp"
#include "geometry/vertex.h"
#include "geometry/edgepoly.h"

using std::string;
using namespace pugi;

class TileWriter : public MosaicWriterBase
{
public:
    TileWriter();

    void setEdgePoly(QTextStream & ts, const EdgePoly & epoly);
    void setTransform(QTextStream & ts, QTransform T);

protected:
    void setVertex(QTextStream & ts,VertexPtr v, QString name);
    void setPoint(QTextStream & ts, QPointF pt, QString name);
};

#endif // FEATUREWRITER_H
