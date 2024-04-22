#pragma once
#ifndef EXTENDED_ROSETTE_H
#define EXTENDED_ROSETTE_H

#include "motifs/rosette.h"
#include "motifs/rosette2.h"
#include "motifs/extender.h"

typedef std::shared_ptr<class Vertex>     VertexPtr;

class ExtendedRosette : public Rosette
{
    friend class MotifView;
public:
    ExtendedRosette(const Motif & fig, int n, qreal q, int s);

    ExtendedRosette(int n, qreal q, int s);

    ExtendedRosette(const ExtendedRosette & other);

    virtual ~ExtendedRosette() override {}

    void    buildMotifMaps() override;

    QString getMotifDesc() override { return "ExtendedRosette";}
    void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "q:" << q << "s" << s
                                                << "preipheralVerts:" << extender. getExtendPeripheralVertices()
                                                << "freeVerts:" << extender.getExtendFreeVertices()
                                                << "boundaryVerts:" << extender. getConnectBoundaryVertices(); }

    Extender & getExtender() { return extender; }

private:
    Extender extender;
};


class ExtendedRosette2 : public Rosette2
{
    friend class MotifView;
public:
    ExtendedRosette2(const Motif & fig, int n, qreal kneeX, qreal kneeY, int s, qreal k, bool c);

    ExtendedRosette2(int n, qreal kneeX, qreal kneeY, int s, qreal k, bool c);

    ExtendedRosette2(const ExtendedRosette2 & other);

    virtual ~ExtendedRosette2() override {}

    void    buildMotifMaps() override;

    QString getMotifDesc() override { return "ExtendedRosette";}
    void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "kneeX:" << kneeX << "kneeY:" << kneeY << "s:" << s << "k:" << k
                                                        << "preipheralVerts:" << extender. getExtendPeripheralVertices()
                                                        << "freeVerts:" << extender.getExtendFreeVertices()
                                                        << "boundaryVerts:" << extender. getConnectBoundaryVertices(); }

    Extender & getExtender() { return extender; }

private:
    Extender extender;
};
#endif

