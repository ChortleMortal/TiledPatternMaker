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

    void    buildMaps() override;

    virtual QString getMotifDesc() override { return "Extended Rosette";}
    Extender & getExtender() { return extender; }

private:
    Extender extender;
};
#endif

