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

    void    buildMaps() override;

    virtual QString getMotifDesc() override { return "Extended Star";}
    Extender & getExtender() { return extender; }

private:
    Extender extender;
};
#endif

