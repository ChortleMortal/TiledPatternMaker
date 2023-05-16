#pragma once
#ifndef EXTENDED_STAR_H
#define EXTENDED_STAR_H

#include "motifs/star.h"
#include "motifs/extender.h"

typedef std::shared_ptr<class Vertex>           VertexPtr;

class ExtendedStar : public Star
{
    friend class MotifView;

public:
    ExtendedStar(int n, qreal d, int s);
    ExtendedStar(const Motif & fig, int n, qreal d, int s);
    ExtendedStar(const ExtendedStar & other);

    virtual ~ExtendedStar() override {}

    Extender & getExtender() { return extender; }

    void    buildMotifMaps() override;

    virtual QString getMotifDesc() override { return "ExtendedStar";}
    virtual void    report()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s
                                               << "preipheralVerts:" << extender. getExtendPeripheralVertices()
                                               << "freeVerts:" << extender.getExtendFreeVertices()
                                               << "boundaryVerts:" << extender. getConnectBoundaryVertices(); }

private:
    Extender extender;
};
#endif

