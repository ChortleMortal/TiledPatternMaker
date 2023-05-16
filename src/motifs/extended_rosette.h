#pragma once
#ifndef EXTENDED_ROSETTE_H
#define EXTENDED_ROSETTE_H

#include "motifs/rosette.h"
#include "motifs/extender.h"

typedef std::shared_ptr<class Vertex>     VertexPtr;

class ExtendedRosette : public Rosette
{
    friend class MotifView;
public:
    ExtendedRosette(const Motif & fig, int n, qreal q, int s, qreal k);

    ExtendedRosette(int n, qreal q, int s, qreal k);

    ExtendedRosette(const ExtendedRosette & other);

    virtual ~ExtendedRosette() override {}

    void    buildMotifMaps() override;

    virtual QString getMotifDesc() override { return "ExtendedRosette";}
    virtual void    report()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "q:" << q << "s" << s << "k" << k
                                                << "preipheralVerts:" << extender. getExtendPeripheralVertices()
                                                << "freeVerts:" << extender.getExtendFreeVertices()
                                                << "boundaryVerts:" << extender. getConnectBoundaryVertices(); }

    Extender & getExtender() { return extender; }

private:
    Extender extender;
};
#endif

