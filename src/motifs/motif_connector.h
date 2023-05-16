#pragma once
#ifndef MOTIF_CONNECTOR_H
#define MOTIF_CONNECTOR_H

#include <memory>
#include <QString>

class RadialMotif;

typedef std::shared_ptr<class Map>        MapPtr;
typedef std::shared_ptr<class Vertex>     VertexPtr;

class MotifConnector
{
public:
    MotifConnector();

    void  connectMotif(RadialMotif * fig);
    qreal computeScale(RadialMotif *  fig);

    void rotateHalf(RadialMotif *  fig);
    void scaleToUnit(RadialMotif * fig);

    void dumpM(QString s,  QMap<VertexPtr, VertexPtr> &movers);

private:
};

#endif
