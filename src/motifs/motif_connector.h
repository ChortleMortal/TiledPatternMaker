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

    void  connectMotif(RadialMotif * motif);
    qreal computeScale(RadialMotif *  motif);

    void rotateHalf(RadialMotif *  motif);
    void scaleToUnit(RadialMotif * motif);

    void dumpM(QString s,  QMap<VertexPtr, VertexPtr> &movers);

private:
};

#endif
