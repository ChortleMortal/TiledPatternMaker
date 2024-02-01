#pragma once
#ifndef EXTENDER_H
#define EXTENDER_H

#include <QtCore>
#include "misc/unique_qvector.h"

typedef std::shared_ptr<class Vertex>     VertexPtr;
typedef std::shared_ptr<class Map>        MapPtr;

class Motif;

class Extender
{
    friend class MotifView;

public:
    Extender();
    Extender(const Extender & other);

    Extender& operator=(const Extender& rhs)  {
        extendPeripheralVertices = rhs.extendPeripheralVertices;
        extendFreeVertices       = rhs.extendFreeVertices;
        connectBoundaryVertices  = rhs.connectBoundaryVertices;
        return *this; }

    void    extend(Motif * motif, QTransform Tr);

    void    setExtendPeripheralVertices(bool extend) { extendPeripheralVertices = extend; }
    void    setExtendFreeVertices(bool extend)       { extendFreeVertices       = extend; }
    void    setConnectBoundaryVertices(bool connect) { connectBoundaryVertices  = connect; }

    bool    getExtendPeripheralVertices() { return extendPeripheralVertices; }
    bool    getExtendFreeVertices() {       return extendFreeVertices; }
    bool    getConnectBoundaryVertices() {  return connectBoundaryVertices; }

protected:
    void    extendPeripheralMap( MapPtr motifMap);
    void    extendFreeMap(       MapPtr motifMap, QTransform unitRotationTr);
    void    connectOuterVertices(MapPtr motifMap);

    qreal   len(VertexPtr v1, VertexPtr v2);

private:
    bool    extendFreeVertices;
    bool    extendPeripheralVertices;
    bool    connectBoundaryVertices;

    Motif * motif;

    UniqueQVector<VertexPtr> newVertices;

};
#endif

