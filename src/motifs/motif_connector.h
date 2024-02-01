#pragma once
#ifndef MOTIF_CONNECTOR_H
#define MOTIF_CONNECTOR_H

#include <QString>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

class RadialMotif;

typedef std::shared_ptr<class Map>        MapPtr;
typedef std::shared_ptr<class Vertex>     VertexPtr;

class MotifConnector
{
public:
    MotifConnector();

    void  connectMotif(RadialMotif * motif, qreal scale);
    qreal computeScale(RadialMotif *  motif);

    void rotateHalf(RadialMotif *  motif);
    void scaleToUnit(RadialMotif * motif);

    void dumpM(QString s,  QMap<VertexPtr, VertexPtr> &movers);

private:
};

#endif
