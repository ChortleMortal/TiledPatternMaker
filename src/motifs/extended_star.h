#pragma once
#ifndef EXTENDED_STAR_H
#define EXTENDED_STAR_H

#include "motifs/star.h"
#include "motifs/star2.h"
#include "motifs/extender.h"

typedef std::shared_ptr<class Vertex>           VertexPtr;

class ExtendedStar : public Star
{
    friend class MotifView;

public:
    ExtendedStar(int n, qreal d, int s);
    ExtendedStar(const Motif & motif, int n, qreal d, int s);
    ExtendedStar(const ExtendedStar & other);

    virtual ~ExtendedStar() override {}

    Extender & getExtender() { return extender; }

    void    buildMotifMaps() override;

    QString getMotifDesc() override { return "ExtendedStar";}
    void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s
                                               << "preipheralVerts:" << extender. getExtendPeripheralVertices()
                                               << "freeVerts:" << extender.getExtendFreeVertices()
                                               << "boundaryVerts:" << extender. getConnectBoundaryVertices(); }

private:
    Extender extender;
};

class ExtendedStar2 : public Star2
{
    friend class MotifView;

public:
    ExtendedStar2(int n, qreal theta, int intersects);
    ExtendedStar2(const Motif & motif, int n, qreal theta, int intersects);
    ExtendedStar2(const ExtendedStar2 & other);

    virtual ~ExtendedStar2() override {}

    Extender & getExtender() { return extender; }

    void    buildMotifMaps() override;

    QString getMotifDesc() override { return "ExtendedStar2";}
    void    dump()       override { qDebug().noquote() << getMotifDesc() << "sides:" << getN() << "d:" << d << "s" << s
                                                        << "preipheralVerts:" << extender. getExtendPeripheralVertices()
                                                        << "freeVerts:" << extender.getExtendFreeVertices()
                                                        << "boundaryVerts:" << extender. getConnectBoundaryVertices(); }

private:
    Extender extender;
};
#endif

