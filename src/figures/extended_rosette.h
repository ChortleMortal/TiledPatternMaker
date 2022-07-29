#ifndef EXTENDED_ROSETTE_H
#define EXTENDED_ROSETTE_H

#include "figures/rosette.h"

typedef std::shared_ptr<class Vertex>     VertexPtr;

class ExtendedRosette : public Rosette
{
    friend class MotifView;
public:
    ExtendedRosette(const Figure & fig,
                    int n, qreal q, int s, qreal k,
                    bool  extendPeripheralVertices = false,
                    bool  extendFreeVertices = true,
                    bool  connectBoundaryVertices = false);

    ExtendedRosette(int n, qreal q, int s, qreal k,
                    bool  extendPeripheralVertices = false,
                    bool  extendFreeVertices = true,
                    bool  connectBoundaryVertices = false);

    virtual ~ExtendedRosette() override {}

    void    buildMaps() override;

    void    setExtendPeripheralVertices(bool extend){ extendPeripheralVertices = extend; }
    void    setExtendFreeVertices(bool extend){ extendFreeVertices = extend; }
    void    setConnectBoundaryVertices(bool connect) { connectBoundaryVertices = connect; }

    bool    getExtendPeripheralVertices(){ return extendPeripheralVertices; }
    bool    getExtendFreeVertices(){ return extendFreeVertices; }
    bool    getConnectBoundaryVertices() { return connectBoundaryVertices; }

    virtual QString getFigureDesc() override { return "Extended Rosette";}

protected:
    void    extendMap();
    void    connectOuterVertices(MapPtr map);

    bool    extendFreeVertices;
    bool    extendPeripheralVertices;
    bool    connectBoundaryVertices;

    qreal   len(VertexPtr v1, VertexPtr v2);

private:

};
#endif

