#pragma once
#ifndef INTERLACE_SIDE_H
#define INTERLACE_SIDE_H

#include <QColor>
#include "model/settings/tristate.h"
#include "model/styles/interlace_casing.h"
#include "model/styles/casing_side.h"

typedef std::shared_ptr<class InterlaceCasing>  InterlaceCasingPtr;

class InterlaceCasingSet;

class InterlaceSide : public CasingSide
{
public:
    InterlaceSide(Casing * parent, eSide side, CNeighboursPtr np, VertexPtr vertex);
    ~InterlaceSide() {}

    void    createSide1_ilace(const EdgePtr & edge, qreal width);
    void    createSide2_ilace(const EdgePtr & edge, qreal width);

    void    setGap(const EdgePtr & edge, qreal gap);

    void    underSide1(const EdgePtr & edge, qreal width);
    void    underSide2(const EdgePtr & edge, qreal width);

    bool    under()         { return _under.get() == Tristate::True; }
    bool    over()          { return _under.get() == Tristate::False; }
    bool    visited()       { return !(_under.get() == Tristate::Unknown);}

    Tristate    _under;

    InterlaceSide & operator =(const InterlaceSide & other);


    void    dump2();

    QString thisSide();

    bool    shadow;

protected:
    QPointF calc_perp(QPointF pt,qreal width);
    qreal 	capGap(QPointF p, QPointF base, qreal gap);
};

#endif
