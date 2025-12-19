#pragma once
#ifndef CASING_NEIGHBOURS_H
#define CASING_NEIGHBOURS_H

#include "model/styles/casing_set.h"
#include "model/styles/casing_side.h"
#include "sys/geometry/neighbours.h"

typedef std::weak_ptr<class Edge>                   wEdgePtr;
typedef std::shared_ptr<class CasingNeighbours>     CNeigboursPtr;
typedef std::weak_ptr<class CasingNeighbours>       WeakCNeighboursPtr;
typedef std::shared_ptr<class NeighbouringCasing>   NeighbouringCasingPtr;

class InterlaceSide;

class NeighbouringCasing
{
public:
    CasingSide * getSide();

    wEdgePtr        wedge;
    Casing     *    casing;
    eSide           side;
};

struct BeforeAndAfterCasings
{
    NeighbouringCasingPtr before;
    NeighbouringCasingPtr after;
};

class CasingNeighbours : public Neighbours
{
public:
    CasingNeighbours(const VertexPtr & vp);
    CasingNeighbours(const Neighbours & np);

    void doUnders();
    void doOvers();

    QString infoWeave();

    Casing *getCasing(EdgePtr edge);
    void    findNeighbouringCasings(CasingSet *cset);

    BeforeAndAfterCasings getBeforeAndAfterCasings(Casing *casing);

    InterlaceSide       * getContinuationOver(const EdgePtr edge);
    NeighbouringCasingPtr getContinuationUnder(const Casing * casing);
    NeighbouringCasingPtr getContinuationOver(const Casing * casing);

    void dump();

    QVector<NeighbouringCasingPtr> casings;
};
#endif // CASING_NEIGHBOURS_H
