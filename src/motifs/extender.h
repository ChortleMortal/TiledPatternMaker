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

    void    extend(Motif * fig, QTransform Tr);

    void    setExtendPeripheralVertices(bool extend) { extendPeripheralVertices = extend; }
    void    setExtendFreeVertices(bool extend)       { extendFreeVertices       = extend; }
    void    setConnectBoundaryVertices(bool connect) { connectBoundaryVertices  = connect; }

    bool    getExtendPeripheralVertices() { return extendPeripheralVertices; }
    bool    getExtendFreeVertices() {       return extendFreeVertices; }
    bool    getConnectBoundaryVertices() {  return connectBoundaryVertices; }

protected:
    void    extendPeripheralMap(MapPtr map);
    void    extendFreeMap(MapPtr map, QTransform Tr);
    void    connectOuterVertices(MapPtr map);

    qreal   len(VertexPtr v1, VertexPtr v2);

private:
    bool    extendFreeVertices;
    bool    extendPeripheralVertices;
    bool    connectBoundaryVertices;

    Motif * fig;

    UniqueQVector<VertexPtr> newVertices;

};
#endif

